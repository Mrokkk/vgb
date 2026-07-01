#ifdef __unix__
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <filesystem>
#include <iterator>
#include <map>
#include <stdio.h>
#include <string>
#include <ucontext.h>
#include <unistd.h>
#include <vector>

#ifdef USE_BACKTRACE
#include <backtrace.h>
#endif

#include <fontconfig/fontconfig.h>
#include <fmt/base.h>
#include <fmt/color.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "config.hpp"
#include "game_boy.hpp"
#include "sys/platform.hpp"
#include "sys/supervision.hpp"
#include "utils/string.hpp"

namespace sys::posix
{

#ifdef USE_BACKTRACE
static backtrace_state* backtraceState;
using BacktraceCreateStateFn = decltype(&backtrace_create_state);
using BacktraceFullFn = decltype(&backtrace_full);
static BacktraceCreateStateFn backtraceCreateState;
static BacktraceFullFn backtraceFull;

struct ProcessMapping final
{
    std::string name;
    uintptr_t   start;
    uintptr_t   end;
};

using ProcessMappings = std::vector<ProcessMapping>;

struct BacktraceContext final
{
    unsigned int    index;
    ProcessMappings maps;
};

static std::string mappingNameFind(const ProcessMappings& mappings, uintptr_t address)
{
    auto mappingIt = std::ranges::find_if(
        mappings,
        [address](const auto& mapping)
        {
            return address >= mapping.start and address < mapping.end;
        });

    if (mappingIt != mappings.end())
    {
        std::string out;
        fmt::format_to(std::back_inserter(out), "{}+{:#x}", mappingIt->name, address - mappingIt->start);
        return out;
    }
    else
    {
        return "??";
    }
}

static int backtraceCallback(void* data, uintptr_t pc, const char* pathname, int lineNumber, const char* function)
{
    using namespace fmt;

    auto ctx = static_cast<BacktraceContext*>(data);

    if (pathname != NULL or function != NULL)
    {
        int status;
        std::string mappingName;

        if (function)
        {
            auto demangled = abi::__cxa_demangle(function, NULL, NULL, &status);
            if (!status)
            {
                function = demangled;
            }
        }
        else
        {
            auto mappingName = mappingNameFind(ctx->maps, pc);
            function = mappingName.c_str();
        }

        println("#{} {} in {} at {}:{}",
            ctx->index,
            styled((void*)pc, fg(terminal_color::blue)),
            styled(function, fg(terminal_color::yellow)),
            styled(pathname, fg(terminal_color::green)),
            lineNumber);
    }
    else
    {
        auto mappingName = mappingNameFind(ctx->maps, pc);
        println("#{} {} in {}",
            ctx->index,
            styled((void*)pc, fg(terminal_color::blue)),
            styled(mappingName, fg(terminal_color::yellow)));
    }

    ctx->index++;

    return 0;
}

static void backtraceErrorCallback(void*, const char* message, int error)
{
    fmt::println("backtrace error[{}]: {}", error, message);
}

template <typename T>
static T hexTo(const std::string& string)
{
    return strtol(string.c_str(), nullptr, 16);
}

static ProcessMappings mappingRead()
{
    // Example of 2 mappings from /proc/self/maps
    // 58868b471000-58868b473000 r--p 00000000 fe:00 12861860                   /usr/bin/cat
    // 702259d69000-702259dae000 rw-p 00000000 00:00 0
    ProcessMappings mappings;

    for (const auto& line : "/proc/self/maps" | utils::readText | utils::splitBy("\n"))
    {
        const auto words = line | utils::splitBy(" ");

        if (words.size() == 0)
        {
            continue;
        }

        const auto addresses = words[0] | utils::splitBy("-");

        mappings.emplace_back(
            ProcessMapping{
                .name = words.size() > 5 ? words[5] : "[anon]",
                .start = hexTo<uintptr_t>(addresses[0]),
                .end = hexTo<uintptr_t>(addresses[1])
            });
    }

    return mappings;
}

static void stacktraceLogInternal()
{
    if (not backtraceFull)
    {
        fmt::println("libbacktrace unavailable; cannot collect stacktrace");
        return;
    }

    auto context = BacktraceContext{
        .index = 0,
        .maps = mappingRead()
    };

    fmt::println("Stacktrace:");

    backtraceFull(backtraceState, 1, backtraceCallback, backtraceErrorCallback, static_cast<void*>(&context));
}

static void crashHandle(int sig, siginfo_t* info, void* context)
{
    stopSupervision();

    auto m = (ucontext_t*)context;

    fmt::println("Received SIG{} at {}",
        sigabbrev_np(sig),
#ifdef __x86_64__
        (void*)m->uc_mcontext.gregs[REG_RIP]
#elifdef __i386__
        (void*)m->uc_mcontext.gregs[REG_EIP]
#endif
    );

    fmt::print("Reason: ");

    switch (sig)
    {
        case SIGILL:
            switch (info->si_code)
            {
                case ILL_ILLOPC:
                    fmt::println("illegal opcode");
                    break;
                case ILL_ILLOPN:
                    fmt::println("illegal operand");
                    break;
                case ILL_ILLADR:
                    fmt::println("illegal addressing mode");
                    break;
                case ILL_ILLTRP:
                    fmt::println("illegal trap");
                    break;
                case ILL_PRVOPC:
                    fmt::println("privileged opcode");
                    break;
                case ILL_PRVREG:
                    fmt::println("privileged register");
                    break;
                case ILL_COPROC:
                    fmt::println("coprocessor error");
                    break;
                case ILL_BADSTK:
                    fmt::println("internal stack error");
                    break;
                default:
                    goto unknown;
            }
            break;
        case SIGSEGV:
            switch (info->si_code)
            {
                case SEGV_MAPERR:
                    fmt::println("{} not mapped to an object", info->si_addr);
                    break;
                case SEGV_ACCERR:
                    fmt::print("invalid permissions for object at {}", info->si_addr);
                    break;
                case SEGV_BNDERR:
                    fmt::print("failed address bound checks for {}", info->si_addr);
                    break;
                case SEGV_PKUERR:
                    fmt::print("access to {} denied by memory protection keys", info->si_addr);
                    break;
                default:
                    goto unknown;
            }
            break;
        case SIGBUS:
            switch (info->si_code)
            {
                case BUS_ADRALN:
                    fmt::println("invalid address alignment at {}", info->si_addr);
                    break;
                case BUS_ADRERR:
                    fmt::println("nonexistent physical address {}", info->si_addr);
                    break;
                case BUS_OBJERR:
                    fmt::println("object-specific hardware error for address {}", info->si_addr);
                    break;
                case BUS_MCEERR_AR:
                    fmt::println("hardware memory error consumed on a machine check");
                    break;
                case BUS_MCEERR_AO:
                    fmt::println("hardware memory error detected in process but not consumed");
                    break;
                default:
                    goto unknown;
            }
            break;

        default:
            switch (info->si_code)
            {
                case SI_USER:
                    if (info->si_pid == getpid())
                    {
                        fmt::println("aborted by supervision");
                    }
                    else
                    {
                        fmt::println("user");
                    }
                    break;
                case SI_KERNEL:
                    fmt::println("kernel");
                    break;
                unknown:
                default:
                    fmt::println("unknown");
                    break;
            }
    }

    stacktraceLogInternal();

    if (sig == SIGABRT)
    {
        abort();
    }
}
#else

static void stacktraceLogInternal()
{
    fmt::println("application built without libbacktrace; cannot collect stacktrace");
}
#endif

static void abort()
{
    auto pid = getpid();
    kill(pid, SIGABRT);
}

static void stacktraceLog(void)
{
    stacktraceLogInternal();
}

static void interruptionHandle(int)
{
    gb.cpu.stop();
}

static bool doesFileExist(const char* pathname)
{
    struct stat buf;
    return stat(pathname, &buf) == 0 and S_ISREG(buf.st_mode);
}

static bool doesDirExist(const char* pathname)
{
    struct stat buf;
    return stat(pathname, &buf) == 0 and S_ISDIR(buf.st_mode);
}

static MaybeMapped mapFileImpl(const char* pathname, bool readOnly)
{
    const int fd = open(pathname, O_RDONLY);

    if (fd == -1) [[unlikely]]
    {
        return std::unexpected(strerror(errno));
    }

    struct stat stat;

    if (fstat(fd, &stat) == -1) [[unlikely]]
    {
        return std::unexpected(strerror(errno));
    }

    auto mapped = mmap(nullptr, stat.st_size, PROT_READ | (readOnly ? 0 : PROT_WRITE), MAP_PRIVATE, fd, 0);

    if (mapped == MAP_FAILED) [[unlikely]]
    {
        return std::unexpected(strerror(errno));
    }

    close(fd);

    return Mapped{
        mapped,
        static_cast<size_t>(stat.st_size)
    };
}

static MaybeError unmapFileImpl(void* ptr, size_t size)
{
    munmap(ptr, size);
    return {};
}

static MaybeError writeToFile(const char* pathname, const void* data, size_t size)
{
    const int fd = open(pathname, O_RDWR | O_CREAT, 0660);

    if (fd == -1) [[unlikely]]
    {
        return std::unexpected(strerror(errno));
    }

    if (write(fd, data, size) == -1) [[unlikely]]
    {
        return std::unexpected(strerror(errno));
    }

    close(fd);

    return true;
}

std::string getConfigDir()
{
    const auto home = std::getenv("HOME");

    if (not home)
    {
        return {};
    }

    std::string path(home);

    path += "/.config/vgb";

    mkdir(path.c_str(), 0755);

    return {path};
}

std::vector<Font> getFonts()
{
    auto config = FcInitLoadConfigAndFonts();
    auto pat = FcPatternCreate();
    auto os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, FC_LANG, nullptr);
    auto fs = FcFontList(config, pat, os);

    if (not fs) [[unlikely]]
    {
        return {};
    }

    std::map<std::string, Font> familyToFonts;

    for (int i = 0; i < fs->nfont; ++i)
    {
        const auto font = fs->fonts[i];
        FcChar8* file = nullptr;
        FcChar8* style = nullptr;
        FcChar8* family = nullptr;

        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch and
            FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch and
            FcPatternGetString(font, FC_STYLE, 0, &style) == FcResultMatch)
        {
            std::filesystem::path fontPath(reinterpret_cast<char*>(file));
            std::string fontStyle(reinterpret_cast<char*>(style));
            std::string fontFamily(reinterpret_cast<char*>(family));

            const auto extension = fontPath.extension();

            if (extension == ".ttf")
            {
                auto& f = familyToFonts[fontFamily];
                f.family = std::move(fontFamily);
                f.styles.emplace_back(std::move(fontPath.native()), std::move(fontStyle));
            }
            else if (extension == ".otf")
            {
                auto& f = familyToFonts[fontFamily];
                f.family = std::move(fontFamily);
                f.styles.emplace_back(std::move(fontPath.native()), std::move(fontStyle));
            }
        }
    }

    FcFontSetDestroy(fs);
    FcPatternDestroy(pat);
    FcObjectSetDestroy(os);
    FcConfigDestroy(config);

    Fonts fonts;

    for (auto& [family, font] : familyToFonts)
    {
        fonts.push_back(font);
    }

    return fonts;
}

void initialize(const Config&)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

#ifdef USE_BACKTRACE
    const auto libbacktrace = dlopen("libbacktrace.so", RTLD_LAZY);

    if (not libbacktrace)
    {
        fprintf(stderr, "cannot load libbacktrace.so\n");
    }
    else
    {
        sa.sa_sigaction = &crashHandle;
        sa.sa_flags = SA_RESETHAND | SA_SIGINFO;

        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGFPE,  &sa, nullptr);
        sigaction(SIGILL,  &sa, nullptr);
        sigaction(SIGSEGV, &sa, nullptr);

        backtraceCreateState = reinterpret_cast<BacktraceCreateStateFn>(dlsym(libbacktrace, "backtrace_create_state"));
        backtraceFull = reinterpret_cast<BacktraceFullFn>(dlsym(libbacktrace, "backtrace_full"));

        if (not backtraceFull or not backtraceCreateState)
        {
            fprintf(stderr, "cannot find backtrace_full or backtrace_create_state\n");
            backtraceFull = nullptr;
            backtraceCreateState = nullptr;
            return;
        }

        backtraceState = backtraceCreateState(NULL, 0, backtraceErrorCallback, NULL);
    }
#endif

    sa.sa_flags = 0;
    sa.sa_handler = &interruptionHandle;

    sigaction(SIGINT, &sa, nullptr);

    platform.abort               = &abort;
    platform.stacktraceLog       = &stacktraceLog;
    platform.writeToFile         = &writeToFile;
    platform.getDefaultConfigDir = &getConfigDir;
    platform.doesDirExist        = &doesDirExist;
    platform.doesFileExist       = &doesFileExist;
    platform.getFonts            = &getFonts;
    platform.mapFileImpl         = &mapFileImpl;
    platform.unmapFileImpl       = &unmapFileImpl;
}

}  // namespace sys::posix
#endif
