#include "system.hpp"

#include <atomic>
#include <backtrace.h>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <iterator>
#include <stdio.h>
#include <string>
#include <thread>
#include <ucontext.h>
#include <unistd.h>
#include <vector>

#include <fmt/base.h>
#include <fmt/color.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "config.hpp"
#include "game_boy.hpp"
#include "utils/string.hpp"

namespace sys
{

static backtrace_state* backtraceState;

struct ProcessMapping
{
    std::string name;
    uintptr_t   start;
    uintptr_t   end;
};

using ProcessMappings = std::vector<ProcessMapping>;

struct BacktraceContext
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

using BacktraceCreateStateFn = decltype(&backtrace_create_state);
using BacktraceFullFn = decltype(&backtrace_full);

BacktraceCreateStateFn backtraceCreateState;
BacktraceFullFn backtraceFull;

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
        fmt::println("#{} {} in {}",
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

void stacktraceLog(void)
{
    stacktraceLogInternal();
}

static std::atomic_int counter;
static std::thread supervisionThread;
static std::atomic_bool stop;

void pingSupervision()
{
    counter++;
}

void stopSupervision()
{
    if (stop)
    {
        return;
    }
    stop = true;
    if (supervisionThread.joinable())
    {
        supervisionThread.join();
    }
}

static void supervision()
{
    using namespace std::chrono_literals;

    int failed = 0;
    while (1)
    {
        std::this_thread::sleep_for(200ms);
        if (stop)
        {
            break;
        }
        if (counter == 0)
        {
            if (++failed == 5)
            {
                auto pid = getpid();
                fmt::println("Main thread ({}) is not responding", pid);
                kill(pid, SIGABRT);
                break;
            }
        }
        else
        {
            counter = 0;
            failed = 0;
        }
    }
}

static void crashHandle(int sig, siginfo_t* info, void* context)
{
    stop = true;

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

static void interruptionHandle(int)
{
    gb.cpu.stop();
}

void initialize(const Config& config)
{
    struct sigaction sa;
    sa.sa_sigaction = &crashHandle;
    sigemptyset(&sa.sa_mask);

    const auto libbacktrace = dlopen("libbacktrace.so", RTLD_LAZY);

    if (not libbacktrace)
    {
        fprintf(stderr, "cannot load libbacktrace.so\n");
    }
    else
    {
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

    sa.sa_flags = 0;
    sa.sa_handler = &interruptionHandle;

    sigaction(SIGINT, &sa, nullptr);

    if (config.useSupervision)
    {
        supervisionThread = std::thread(&supervision);
        atexit(&stopSupervision);
    }
}

MaybeString readLineFromStdin(const std::string_view& prompt)
{
    rl_catch_signals = 0;
    auto buffer = readline(prompt.data());

    if (not buffer)
    {
        return "";
    }

    auto res = std::string(buffer);

    if (*buffer == '\n' or *buffer == '\0')
    {
        free(buffer);
        return "\n";
    }

    add_history(buffer);

    free(buffer);

    return res;
}

bool doesFileExist(const char* pathname)
{
    struct stat buf;
    return stat(pathname, &buf) == 0 and S_ISREG(buf.st_mode);
}

bool doesDirExist(const char* pathname)
{
    struct stat buf;
    return stat(pathname, &buf) == 0 and S_ISDIR(buf.st_mode);
}

MappedFile::MappedFile(bool readonly, void* ptr, size_t size)
    : mReadonly(readonly)
    , mPtr(ptr)
    , mSize(size)
{
}

MappedFile::~MappedFile()
{
    if (mPtr)
    {
        munmap(mPtr, mSize);
    }
}

MappedFile::MappedFile(MappedFile&& other)
    : mReadonly(other.mReadonly)
    , mPtr(other.mPtr)
    , mSize(other.mSize)
{
    other.mPtr = nullptr;
    other.mSize = 0;
}

MappedFile& MappedFile::operator=(MappedFile&& other)
{
    mReadonly = other.mReadonly;
    mPtr = other.mPtr;
    mSize = other.mSize;
    other.mPtr = nullptr;
    other.mSize = 0;
    return *this;
}

MaybeMappedFile mapFile(const char* pathname, bool readOnly)
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

    return MappedFile{
        readOnly,
        mapped,
        static_cast<size_t>(stat.st_size)
    };
}

std::expected<bool, std::string> saveToFile(const char* pathname, const void* data, size_t size)
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

}  // namespace sys
