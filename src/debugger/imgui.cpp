#define IMGUI_DEFINE_MATH_OPERATORS
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>
#include <strings.h>
#include <utility>

#include <fmt/fmt_ext.h>
#include <imgui.h>
#include <imgui_ext.h>
#include <imgui_internal.h>
#include <imgui_memory_editor/imgui_memory_editor.h>

#include "core/ini_serializer.hpp"
#include "core/logger.hpp"
#include "core/logger_reader.hpp"
#include "core/severity.hpp"
#include "cpu/isa/decoder.hpp"
#include "cpu/sm83.hpp"
#include "debugger/context.hpp"
#include "game_boy.hpp"
#include "interpreter/interpreter.hpp"
#include "memory/memory_map.hpp"
#include "ppu.hpp"
#include "save_manager.hpp"
#include "sys/font.hpp"
#include "sys/platform.hpp"
#include "utils/inline.hpp"
#include "utils/unique_ptr.hpp"
#include "utils/units.hpp"

namespace debugger
{

namespace
{

struct BitEntry final
{
    uint8_t     shift;
    uint8_t     mask;
    const char* name;
};

struct IoEntry final
{
    uint8_t     addr;
    const char* name;
    const char* desc;
    BitEntry    bitEntries[8];
};

}  // namespace

#define IO_ENTRY(ADDR, NAME, DESC, ...) \
    { \
        .addr = ADDR, \
        .name = NAME, \
        .desc = DESC, \
        .bitEntries = __VA_ARGS__ \
    }

#define INVALID_IO_ENTRY(ADDR) \
    { \
        .addr = ADDR, \
        .name = "--", \
        .desc = "N/A" \
    }

IoEntry ioEntries[] = {
    IO_ENTRY(0x00, "JOYP", "JoypadA", {
        {
            .shift = 0,
            .mask = 1,
            .name = "A / Right",
        },
        {
            .shift = 1,
            .mask = 1,
            .name = "B / Left",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Select / Up",
        },
        {
            .shift = 3,
            .mask = 1,
            .name = "Start / Down",
        },
        {
            .shift = 4,
            .mask = 1,
            .name = "Select d-pad",
        },
        {
            .shift = 5,
            .mask = 1,
            .name = "Select buttons",
        },
    }),
    IO_ENTRY(0x01, "SB", "Serial transfer data", {
        {
            .shift = 0,
            .mask = 1,
            .name = "Clock select"
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Clock speed",
        },
        {
            .shift = 7,
            .mask = 1,
            .name = "Transfer enable",
        },
    }),
    IO_ENTRY(0x02, "SC", "Serial transfer control", {
        {
            .shift = 0,
            .mask = 1,
            .name = "Clock select"
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Clock speed",
        },
        {
            .shift = 7,
            .mask = 1,
            .name = "Transfer enable",
        },
    }),
    IO_ENTRY(0x04, "DIV", "Divider register", {}),
    IO_ENTRY(0x05, "TIMA", "Timer counter", {}),
    IO_ENTRY(0x06, "TMA", "Timer modulo", {}),
    IO_ENTRY(0x07, "TAC", "Timer control", {
        {
            .shift = 0,
            .mask = 3,
            .name = "Clock select",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Enable",
        },
    }),
    IO_ENTRY(0x0f, "IF", "Interrupt flag", {
        {
            .shift = 0,
            .mask = 1,
            .name = "VBlank",
        },
        {
            .shift = 1,
            .mask = 1,
            .name = "LCD",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Timer",
        },
        {
            .shift = 3,
            .mask = 1,
            .name = "Serial",
        },
        {
            .shift = 4,
            .mask = 1,
            .name = "Joypad",
        },
    }),
    IO_ENTRY(0x40, "LCDC", "LCD control", {
        {
            .shift = 0,
            .mask = 1,
            .name = "BG & Window enable / priority",
        },
        {
            .shift = 1,
            .mask = 1,
            .name = "OBJ enable",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "OBJ size",
        },
        {
            .shift = 3,
            .mask = 1,
            .name = "BG tile map",
        },
        {
            .shift = 4,
            .mask = 1,
            .name = "BG & Window tiles",
        },
        {
            .shift = 5,
            .mask = 1,
            .name = "Window enable",
        },
        {
            .shift = 6,
            .mask = 1,
            .name = "Window tile map",
        },
        {
            .shift = 7,
            .mask = 1,
            .name = "LCD & PPU enable",
        },
    }),
    IO_ENTRY(0x41, "STAT", "LCD status", {
        {
            .shift = 0,
            .mask = 3,
            .name = "PPU mode",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "LYC == LY",
        },
        {
            .shift = 3,
            .mask = 1,
            .name = "Mode 0 int select",
        },
        {
            .shift = 4,
            .mask = 1,
            .name = "Mode 1 int select",
        },
        {
            .shift = 5,
            .mask = 1,
            .name = "Mode 2 int select",
        },
        {
            .shift = 6,
            .mask = 1,
            .name = "LYC int select",
        },
    }),
    IO_ENTRY(0x42, "SCY", "Background viewport Y position", {}),
    IO_ENTRY(0x43, "SCX", "Background viewport X position", {}),
    IO_ENTRY(0x44, "LY", "LCD Y coordinate", {}),
    IO_ENTRY(0x45, "LYC", "LY compare", {}),
    IO_ENTRY(0x46, "DMA", "OAM DMA source address & start", {}),
    IO_ENTRY(0x47, "BGP", "BG palette data", {
        {
            .shift = 0,
            .mask = 3,
            .name = "ID 0",
        },
        {
            .shift = 2,
            .mask = 3,
            .name = "ID 1",
        },
        {
            .shift = 4,
            .mask = 3,
            .name = "ID 2",
        },
        {
            .shift = 6,
            .mask = 3,
            .name = "ID 3",
        },
    }),
    IO_ENTRY(0x48, "OBP0", "OBJ palette 0 data", {
        {
            .shift = 2,
            .mask = 3,
            .name = "ID 1",
        },
        {
            .shift = 4,
            .mask = 3,
            .name = "ID 2",
        },
        {
            .shift = 6,
            .mask = 3,
            .name = "ID 3",
        },
    }),
    IO_ENTRY(0x49, "OBP1", "OBJ palette 1 data", {
        {
            .shift = 2,
            .mask = 3,
            .name = "ID 1",
        },
        {
            .shift = 4,
            .mask = 3,
            .name = "ID 2",
        },
        {
            .shift = 6,
            .mask = 3,
            .name = "ID 3",
        },
    }),
    IO_ENTRY(0x4a, "WY", "Window Y position", {}),
    IO_ENTRY(0x4b, "WX", "Window X position plus 7", {}),
    IO_ENTRY(0x50, "BANK", "Boot ROM mapping control", {}),
    IO_ENTRY(0xff, "IE", "Interrupt enable", {
        {
            .shift = 0,
            .mask = 1,
            .name = "VBlank",
        },
        {
            .shift = 1,
            .mask = 1,
            .name = "LCD",
        },
        {
            .shift = 2,
            .mask = 1,
            .name = "Timer",
        },
        {
            .shift = 3,
            .mask = 1,
            .name = "Serial",
        },
        {
            .shift = 4,
            .mask = 1,
            .name = "Joypad",
        },
    }),
};

static utils::UniquePtr<MemoryEditor> memEditor;
static std::string buffer;
static std::vector<uint32_t> offsets;

static ImU8 readMemory(const ImU8*, size_t addr, void*)
{
    return gb.cpu.mem.load8((uint16_t)addr);
}

static void writeMemory(ImU8*, size_t, ImU8, void*)
{
}

ALWAYS_INLINE static ImVec2 scaleToRatio(const ImVec2& vec, int ratioX, int ratioY)
{
    ImVec2 res = vec;
    if (vec.x * ratioY > vec.y * ratioX)
    {
        res.x = vec.y * ratioX / ratioY;
    }
    else
    {
        res.y = vec.x * ratioY / ratioX;
    }
    return res;
}

static void onLog(Context& ctx, const core::LogEntry& entry)
{
    if (static_cast<int>(entry.severity) > static_cast<int>(core::Severity::info))
    {
        ctx.gui.messages.push_back(Message{
            .severity = entry.severity,
            .time = 0,
            .text = entry.message,
        });
    }

    char timeBuf[32];

    struct tm tmInfo;
    localtime_r(&entry.time, &tmInfo);

    strftime(timeBuf, sizeof(timeBuf), "%F %T", &tmInfo);

    offsets.push_back(buffer.size());

    if (entry.header)
    {
        buffer += fmt::format_to_string("[{}] [{}] {}: {}\n",
            timeBuf,
            entry.header,
            entry.location.func,
            entry.message);
    }
    else
    {
        buffer += fmt::format_to_string("[{}] {}: {}\n",
            timeBuf,
            entry.location.func,
            entry.message);
    }
}

static void execute(Context&, std::string command)
{
    auto result = interpreter::exectuteCommand(std::move(command));

    if (not result) [[unlikely]]
    {
        core::logger.error().buffer() = std::move(result.error());
    }
}

static void* openIni(ImGuiContext*, ImGuiSettingsHandler* s, const char*)
{
    return s->UserData;
}

static void readIniLine(ImGuiContext*, ImGuiSettingsHandler*, void*, const char* line)
{
    auto result = core::IniSerializer::deserializeLine(line);
    if (not result)
    {
        core::logger.error().write("INI error: {}", result.error());
    }
}

static void writeIni(ImGuiContext*, ImGuiSettingsHandler* s, ImGuiTextBuffer* buf)
{
    buf->reserve(200);
    buf->appendf("[%s][Data]\n", s->TypeName);
    auto str = core::IniSerializer::serialize();
    buf->append(str.c_str(), str.c_str() + str.size());
}

void initImGui(Context& ctx)
{
    ctx.gui.messageTime = 180;
    ctx.gui.messageFadeOutTime = 60;
    ctx.gui.fonts = sys::getFonts();
    ctx.gui.fontSize = 16;

    memEditor = utils::makeUnique<MemoryEditor>();
    memEditor->ReadFn          = &readMemory;
    memEditor->WriteFn         = &writeMemory;
    memEditor->Cols            = 8;
    memEditor->OptShowOptions  = false;
    memEditor->OptUpperCaseHex = false;
    memEditor->Open            = false;

    core::IniSerializer::registerData("memEditorWindow", memEditor->Open);

    {
        std::filesystem::path iniPath(sys::getDefaultConfigDir());
        iniPath /= "gui.ini";
        ctx.gui.iniPath = std::move(iniPath.native());
    }

    ImGuiSettingsHandler iniHandler;
    iniHandler.TypeName = "GUI";
    iniHandler.TypeHash = ImHashStr("GUI");
    iniHandler.ReadOpenFn = &openIni;
    iniHandler.ReadLineFn = &readIniLine;
    iniHandler.WriteAllFn = &writeIni;
    iniHandler.UserData = &ctx.gui;
    ImGui::AddSettingsHandler(&iniHandler);

    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::LoadIniSettingsFromDisk(ctx.gui.iniPath.c_str());

    ctx.gui.defaultFont = io.FontDefault;

    if (ctx.gui.fontFamily.get().empty())
    {
        ctx.gui.fontFamily = "(default)";
        ctx.gui.fontStyle = "(default)";
        ctx.gui.currentFont = nullptr;
        ctx.gui.currentFontStyle = nullptr;
    }
    else
    {
        for (const auto& font : ctx.gui.fonts)
        {
            if (font.family == ctx.gui.fontFamily.get())
            {
                for (const auto& style : font.styles)
                {
                    if (style.name == ctx.gui.fontStyle.get())
                    {
                        io.FontDefault = io.Fonts->AddFontFromFileTTF(style.path.c_str());
                        ctx.gui.currentFont = &font;
                        ctx.gui.currentFontStyle = &style;
                    }
                }
                if (not ctx.gui.currentFontStyle)
                {
                    ctx.gui.currentFontStyle = &font.styles[0];
                }
            }
        }
    }

    auto& style = ImGui::GetStyle();
    style.FontSizeBase      = ctx.gui.fontSize;
    style.WindowBorderSize  = 0;
    style.FrameRounding     = 5.0f;
    style.ScrollbarRounding = 5.0f;

    auto logCallback =
        [&ctx](const core::LogEntry& e)
        {
            onLog(ctx, e);
        };

    core::loggerReader.forEachLogEntry(logCallback);
    core::loggerReader.onLog(std::move(logCallback));
}

void deinitImGui(Context& ctx)
{
    ImGui::SaveIniSettingsToDisk(ctx.gui.iniPath.c_str());
}

#define BOOL_MENU_ITEM(NAME, SHRTC, BOOL) \
    do \
    if (ImGui::MenuItem(NAME, SHRTC, BOOL)) \
    { \
        BOOL ^= true; \
    } \
    while (0)

#define SPEED_MENU_ITEM(SPEED) \
    do if (ImGui::MenuItem(#SPEED "x", nullptr, gb.speedMultiplier == SPEED)) \
    { \
        gb.speedMultiplier = SPEED; \
    } \
    while (0)

ALWAYS_INLINE static void drawMenuBar(Context& ctx)
{
    auto mainMenu = ImGui::CreateMainMenuBar();

    if (not mainMenu)
    {
        return;
    }

    if (auto menu = ImGui::CreateMenu("File"))
    {
        if (ImGui::MenuItem("Configuration"))
        {
            ctx.gui.configWindow = true;
        }
        if (ImGui::MenuItem("Quit"))
        {
            gb.cpu.exc.reportUserInterruption();
        }
    }
    if (auto menu = ImGui::CreateMenu("Emulation"))
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            if (ImGui::MenuItem("Start"))
            {
                gb.start();
            }
        }
        else
        {
            if (ImGui::MenuItem("Stop"))
            {
                gb.stop();
            }
        }

        if (auto menu = ImGui::CreateMenu("Speed"))
        {
            SPEED_MENU_ITEM(1);
            SPEED_MENU_ITEM(2);
            SPEED_MENU_ITEM(3);
            SPEED_MENU_ITEM(4);
            SPEED_MENU_ITEM(5);
        }
    }

    if (auto menu = ImGui::CreateMenu("View"))
    {
        BOOL_MENU_ITEM("Emulation", nullptr, ctx.gui.emulationWindow);
        BOOL_MENU_ITEM("Cartridge", nullptr, ctx.gui.cartridgeWindow);
        BOOL_MENU_ITEM("CPU", nullptr, ctx.gui.cpuWindow);
        BOOL_MENU_ITEM("Memory", nullptr, memEditor->Open);
        BOOL_MENU_ITEM("Disassembly window", nullptr, ctx.gui.disassemblyWindow);
        BOOL_MENU_ITEM("Callstack window", nullptr, ctx.gui.callstackWindow);
        BOOL_MENU_ITEM("IO", nullptr, ctx.gui.ioWindow);
        BOOL_MENU_ITEM("Map", nullptr, ctx.gui.mapWindow);
        BOOL_MENU_ITEM("Console", "`", ctx.gui.consoleWindow);
        BOOL_MENU_ITEM("Game", nullptr, ctx.gui.gameWindow);
        BOOL_MENU_ITEM("Log", nullptr, ctx.gui.logWindow);
        BOOL_MENU_ITEM("Style Editor", nullptr, ctx.gui.styleEditorWindow);
        BOOL_MENU_ITEM("Demo", nullptr, ctx.gui.demoWindow);
    }
}

ALWAYS_INLINE static void drawLcd(Context&, unsigned int gameTextureId)
{
    auto window = ImGui::CreateWindow("LCD");

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text(ImGui::IsWindowFocused() ? "%0.1f FPS; speed: %ux" : "%0.1f FPS; speed: %ux  (click to focus)", ImGui::GetIO().Framerate, gb.speedMultiplier);
    const auto viewportSize = ImGui::GetContentRegionAvail();
    const auto imageSize = scaleToRatio(viewportSize, GB_LCD_RESX, GB_LCD_RESY);
    ImGui::SetCursorPos((viewportSize - imageSize) * 0.5 + ImGui::GetCursorPos());
    ImGui::Image(static_cast<ImTextureID>(gameTextureId), imageSize);
    gb.inputEnabled = ImGui::IsWindowFocused();
}

#define CREATE_WINDOW(NAME, VARIABLE) \
    if (not VARIABLE) \
    { \
        return; \
    } \
    auto window = ImGui::CreateWindow(NAME, &VARIABLE.get()); \
    if (not window) [[unlikely]] \
    { \
        return; \
    }

ALWAYS_INLINE static void drawCartridgeWindow(Context& ctx)
{
    CREATE_WINDOW("Cartridge", ctx.gui.cartridgeWindow);

    ImGui::Text("Path: %s", gb.config.cartridgePath.c_str());
    ImGui::Text("Title: %s", gb.cartridge.getTitle());
    ImGui::Text("Type: %s", gb.cartridge.getType());

    auto romSize = utils::humanReadable(gb.cartridge.getRomSize());
    auto ramSize = utils::humanReadable(gb.cartridge.getRamSize());

    ImGui::Text("ROM size: %zu %s", romSize.value, romSize.unit);
    ImGui::Text("RAM size: %zu %s", ramSize.value, ramSize.unit);
}

ALWAYS_INLINE static void drawCpuWindow(Context& ctx)
{
    CREATE_WINDOW("CPU", ctx.gui.cpuWindow);

    ImGui::Text("State: %s", gb.cpu.state == cpu::SM83::State::Halted ? "halted" : "running");

    ImGui::SeparatorText("Registers");
    {
        ImGui::Text("AF:  %04x; {c: %u, h: %u; n: %u, z: %u}",
            gb.cpu.af.get(),
            gb.cpu.f.c,
            gb.cpu.f.h,
            gb.cpu.f.n,
            gb.cpu.f.z);
        ImGui::Text("BC:  %04x", gb.cpu.bc.get());
        ImGui::Text("DE:  %04x", gb.cpu.de.get());
        ImGui::Text("HL:  %04x", gb.cpu.hl.get());
        ImGui::Text("SP:  %04x", gb.cpu.sp.get());
        ImGui::Text("PC:  %04x", gb.cpu.pc.get());
        ImGui::Text("IME: %01x", gb.cpu.ime.get());
        ImGui::Text("IE:  %02x", gb.cpu.ie.get());
        ImGui::Text("IF:  %02x", gb.cpu.$if.get());
    }

#define PRINT_IRQ(INT) \
    ImGui::Text(#INT ": %s", cpu::checkIrq(cpu::IRQ::INT, gb.cpu.ie) ? "enabled" : "disabled"); \
    if (cpu::checkIrq(cpu::IRQ::INT, gb.cpu.$if)) \
    { \
        ImGui::SameLineText("/ requested"); \
    }

    ImGui::SeparatorText("Interrupts");
    {
        PRINT_IRQ(VBlank);
        PRINT_IRQ(LCD);
        PRINT_IRQ(Timer);
        PRINT_IRQ(Serial);
        PRINT_IRQ(Joypad);
    }

    ImGui::SeparatorText("Stats");
    {
        auto deltaTime = ImGui::GetIO().DeltaTime;
        if (ctx.gui.counter++ == 60)
        {
            ctx.gui.counter = 0;
            ctx.gui.ips = ctx.gui.sumIps / 60;
            ctx.gui.mhz = ctx.gui.sumMhz / 60;
            ctx.gui.sumIps = 0;
            ctx.gui.sumMhz = 0;
        }

        ctx.gui.sumIps += (gb.cpu.instructions - ctx.gui.prevInstructions) / deltaTime;
        ctx.gui.sumMhz += (gb.cpu.cycles - ctx.gui.prevCycles) / deltaTime;
        ctx.gui.prevInstructions = gb.cpu.instructions;
        ctx.gui.prevCycles = gb.cpu.cycles;

        ImGui::Text("Instructions: %zu", gb.cpu.instructions);
        ImGui::SameLineText("(%.02f kIPS)", ctx.gui.ips / 1000);
        ImGui::Text("T-cycles: %zu", gb.cpu.cycles);
        ImGui::SameLineText("(%.03f MHz)", ctx.gui.mhz / 1000000);
    }
}

ALWAYS_INLINE static void drawMemoryWindow(Context& ctx)
{
    if (not memEditor->Open)
    {
        return;
    }

    constexpr auto addressSpace = memory::Map::ADDRESS_SPACE_SIZE;

    MemoryEditor::Sizes s;
    memEditor->CalcSizes(s, addressSpace, 0);
    ImGui::SetNextWindowSize(ImVec2(s.WindowWidth, s.WindowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));

    auto window = ImGui::CreateWindow("Memory", &memEditor->Open, ImGuiWindowFlags_NoScrollbar);

    if (not window) [[unlikely]]
    {
        return;
    }

    if (ImGui::InputText("Go to address", ctx.gui.addrBuffer, sizeof(ctx.gui.addrBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        char* tmp;
        auto addr = strtoul(ctx.gui.addrBuffer, &tmp, 16);
        memEditor->GotoAddr = addr;
    }

    ImGui::Text("Go to: ");

    if (ImGui::SameLineButton("PC"))
    {
        memEditor->GotoAddr = gb.cpu.pc;
    }

    if (ImGui::SameLineButton("SP"))
    {
        memEditor->GotoAddr = gb.cpu.sp;
    }

    if (ImGui::SameLineButton("VRAM"))
    {
        memEditor->GotoAddr = memory::Map::VRAM.start;
    }

    if (ImGui::SameLineButton("IO"))
    {
        memEditor->GotoAddr = memory::Map::IO.start;
    }

    memEditor->DrawContents(nullptr, addressSpace, 0);

    if (memEditor->ContentsWidthChanged)
    {
        memEditor->CalcSizes(s, addressSpace, 0);
        ImGui::SetWindowSize(ImVec2(s.WindowWidth, ImGui::GetWindowSize().y));
    }
}

ALWAYS_INLINE static void drawConsoleWindow(Context& ctx)
{
    CREATE_WINDOW("Console", ctx.gui.consoleWindow);

    auto scroll = ImGui::CreateChild("##Scroll");

    if (not scroll)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0, 1});

    for (const auto& line : ctx.console.lines)
    {
        ImGui::TextUnformatted(line.c_str());
    }

    ImGui::Text("%s ", ctx.console.prompt.c_str());
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});
    ImGui::PushItemWidth(-1);

    if (ctx.gui.commandEntered)
    {
        ImGui::SetScrollFromPosY(1.0f);
        ctx.gui.commandEntered = false;
    }

    if (ImGui::InputText("#CmdLine", ctx.gui.lineBuffer, sizeof(ctx.gui.lineBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        ctx.gui.commandEntered = true;

        std::string command(ctx.gui.lineBuffer[0] ? ctx.gui.lineBuffer : ctx.prevLine);
        ctx.gui.lineBuffer[0] = 0;

        ctx.console.writeLine("{} {}", ctx.console.prompt, command);
        auto result = interpreter::exectuteCommand(command);

        if (not result) [[unlikely]]
        {
            ctx.console.addLine(std::move(result.error()));
        }
        else
        {
            ctx.prevLine = std::move(command);
        }

        ctx.gui.focusCmdLine = true;
    }

    if (ctx.gui.focusCmdLine)
    {
        ImGui::SetKeyboardFocusHere(-1);
        ctx.gui.focusCmdLine = false;
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ctx.gui.focusCmdLine = ImGui::IsMouseClicked(ImGuiMouseButton_Left) and ImGui::IsWindowHovered();

    if (ImGui::IsWindowFocused() and ImGui::IsKeyReleased(ImGuiKey_C) and ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        ctx.console.addLine("Interrupted");
        gb.stop();
    }
}

ALWAYS_INLINE static void drawMapWindow(Context& ctx)
{
    CREATE_WINDOW("Map", ctx.gui.mapWindow);
    ImGui::Checkbox("Show SCX/SCY window", &ctx.gui.showScxScy.get());
    const auto viewportSize = ImGui::GetContentRegionAvail();
    const auto imageSize = scaleToRatio(viewportSize, 1, 1);
    ImGui::SetCursorPos((viewportSize - imageSize) * 0.5 + ImGui::GetCursorPos());
    ImGui::Image(static_cast<ImTextureID>(sys::platform.renderer->renderMap(ctx.gui.showScxScy)), imageSize);
}

ALWAYS_INLINE static void drawStyleEditorWindow(Context& ctx)
{
    CREATE_WINDOW("Style Editor", ctx.gui.styleEditorWindow);
    ImGui::ShowStyleEditor();
}

ALWAYS_INLINE static void drawIoWindow(Context& ctx)
{
    CREATE_WINDOW("IO", ctx.gui.ioWindow);

    ImGui::InputText("Filter", ctx.gui.ioFilterBuffer, sizeof(ctx.gui.ioFilterBuffer));

    constexpr auto tableFlags
        = ImGuiTableFlags_Borders
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_NoBordersInBody;

    constexpr auto parentFlags
        = ImGuiTreeNodeFlags_SpanAllColumns
        | ImGuiTreeNodeFlags_DrawLinesFull;

    constexpr auto leafFlags
        = parentFlags
        | ImGuiTreeNodeFlags_Leaf
        | ImGuiTreeNodeFlags_Bullet
        | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    if (auto table = ImGui::CreateTable("IOTable", 3, tableFlags))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Address / Bit");
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoHide);
        ImGui::TableHeadersRow();

        const auto ioFilterLen = strlen(ctx.gui.ioFilterBuffer);

        for (const auto& entry : ioEntries)
        {
            if (ctx.gui.ioFilterBuffer[0] and strncasecmp(entry.name, ctx.gui.ioFilterBuffer, ioFilterLen))
            {
                continue;
            }

            const auto value = gb.cpu.mem.load8(0xff00 + entry.addr);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (entry.bitEntries[0].name)
            {
                bool open = ImGui::TreeNodeEx(entry.name, parentFlags);
                ImGui::TableNextColumn();
                ImGui::Text("%x", 0xff00 + entry.addr);
                ImGui::TableNextColumn();
                ImGui::Text("%x", value);

                if (open)
                {
                    for (const auto& bit : entry.bitEntries)
                    {
                        if (not bit.name)
                        {
                            break;
                        }

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TreeNodeEx(bit.name, leafFlags);
                        ImGui::TableNextColumn();
                        ImGui::Text("%u", bit.shift);
                        ImGui::TableNextColumn();
                        ImGui::Text("%x", (value >> bit.shift) & bit.mask);
                    }
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::TreeNodeEx(entry.name, leafFlags);
                ImGui::TableNextColumn();
                ImGui::Text("%x", 0xff00 + entry.addr);
                ImGui::TableNextColumn();
                ImGui::Text("%x", value);
            }
        }
    }
}

ALWAYS_INLINE static void drawGameWindow(Context& ctx)
{
    CREATE_WINDOW("Game", ctx.gui.gameWindow);

    if (not ctx.game)
    {
        ImGui::TextDisabled("Not supported game");
        return;
    }

    ctx.game->drawUi();
}

ALWAYS_INLINE static void drawLogWindow(Context& ctx)
{
    if (not ctx.gui.logWindow)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});

    if (auto window = ImGui::CreateWindow("Log", &ctx.gui.logWindow.get())) [[likely]]
    {
        ImGui::InputTextMultiline("##Log", buffer.data(), buffer.size() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

ALWAYS_INLINE static void drawEmulationWindow(Context& ctx)
{
    if (ImGui::IsKeyReleased(ImGuiKey_F5))
    {
        SaveManager::quickSave();
    }
    else if (ImGui::IsKeyReleased(ImGuiKey_F9))
    {
        SaveManager::quickLoad();
    }

    CREATE_WINDOW("Emulation", ctx.gui.emulationWindow);

    ImGui::SeparatorText("Emulation");
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            if (ImGui::Button("Start"))
            {
                gb.start();
            }
        }
        else
        {
            if (ImGui::Button("Stop"))
            {
                gb.stop();
            }
        }

        if (ImGui::SameLineButton("Reset"))
        {
            execute(ctx, "reset");
        }

        ImGui::SliderInt("Speed", reinterpret_cast<int*>(&gb.speedMultiplier), 1, 20);

        if (ImGui::Button("Step"))
        {
            execute(ctx, "step");
        }
    }

    ImGui::SeparatorText("Save RAM");
    {
        const bool dirty = gb.cartridge.isRamDirty();
        if (not dirty)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        if (ImGui::Button("Save"))
        {
            gb.saveRam();
        }
        if (not dirty)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
    }

    ImGui::SeparatorText("Saves");
    {
        ImGui::Text("Saves directory: %s", SaveManager::getSaveDir().c_str());
        if (ImGui::Button("Quick save (F5)"))
        {
            SaveManager::quickSave();
        }
        if (ImGui::Button("Quick load (F9)"))
        {
            SaveManager::quickLoad();
        }
        ImGui::SeparatorText("Saves");
        {
            unsigned i = 0;
            for (const auto& save : SaveManager::getSaves())
            {
                ImGui::Text("%u: %s", i, save.name.c_str());
            }
        }
    }
}

ALWAYS_INLINE static void drawDisassemblyWindow(Context& ctx)
{
    CREATE_WINDOW("Disassembly", ctx.gui.disassemblyWindow);

    if (ctx.gb.state == GameBoy::State::Running)
    {
        ImGui::TextDisabled("--");
        return;
    }

    auto disassembleCtx = cpu::isa::DisassembleContext::create(ctx.cpu, ctx.cpu.pc);

    constexpr auto tableFlags
        = ImGuiTableFlags_Borders
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_NoBordersInBody;

    if (auto table = ImGui::CreateTable("Disassembly", 3, tableFlags))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Bytes");
        ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_NoHide);
        ImGui::TableHeadersRow();

        for (int i = 0; i < 10; ++i)
        {
            auto addr = disassembleCtx.pc;
            cpu::isa::disassemble(disassembleCtx);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%04x", addr);
            ImGui::TableNextColumn();
            for (size_t i = 0; i < disassembleCtx.data.bytes; ++i)
            {
                ImGui::SameLineText("%02x", disassembleCtx.data.data[i]);
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(disassembleCtx.disassembled.c_str());
        }
    }
}

#define MEMORY_RANGE(RANGE) \
    case memory::Map::RANGE.start ... memory::Map::RANGE.end - 1

static void drawCallstackEntry(Context& ctx, int id, uint16_t bank, uint16_t addr)
{
    const Symbol* sym = nullptr;
    char buf[32];
    buf[0] = 0;
    switch (addr)
    {
        MEMORY_RANGE(BOOT_ROM):
            if (ctx.cpu.mem.isBootRomEnabled())
            {
                sprintf(buf, "BOOTROM ");
                break;
            }
            [[fallthrough]];

        MEMORY_RANGE(ROM):
            if (addr < 0x4000)
            {
                sprintf(buf, "ROM #00:");
                sym = ctx.symbols[addr];
            }
            else
            {
                sprintf(buf, "ROM #%02u:", bank + 1);
                sym = ctx.symbols[bank + 1, addr];
            }
            break;

        MEMORY_RANGE(HRAM):
            sprintf(buf, "HRAM    ");
            sym = ctx.symbols[addr];
            break;
    }
    ImGui::Text("%- 4i [%s%04x] %s", id, buf, addr, sym ? sym->name.c_str() : "??");
}

ALWAYS_INLINE static void drawCallstackWindow(Context& ctx)
{
    CREATE_WINDOW("Callstack", ctx.gui.callstackWindow);

    if (ctx.gb.state == GameBoy::State::Running)
    {
        ImGui::TextDisabled("--");
        return;
    }

    int id = 0;
    drawCallstackEntry(ctx, id++, ctx.gb.cartridge.getRomBank(), ctx.cpu.pc);
    for (int i = ctx.cpu.callstack.index - 1; i >= 0; --i)
    {
        auto& entry = ctx.cpu.callstack.data[i];
        drawCallstackEntry(ctx, id++, entry.romBank, entry.ret);
    }
}

static void setFont(Context& ctx, ImGuiIO& io, const sys::Font& font, const sys::FontStyle& fontStyle)
{
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontStyle.path.c_str());
    ctx.gui.fontFamily = font.family;
    ctx.gui.fontStyle = fontStyle.name;
    ctx.gui.currentFont = &font;
    ctx.gui.currentFontStyle = &fontStyle;
}

static void setDefaultFont(Context& ctx, ImGuiIO& io)
{
    io.FontDefault = static_cast<ImFont*>(ctx.gui.defaultFont);
    ctx.gui.fontFamily = "(default)";
    ctx.gui.fontStyle = "(default)";
    ctx.gui.currentFont = nullptr;
    ctx.gui.currentFontStyle = nullptr;
}

static void drawViewConfig(Context& ctx, ImGuiIO& io)
{
    if (auto _ = ImGui::CreateCombo("Font family", ctx.gui.fontFamily.get().c_str(), ImGuiComboFlags_HeightLargest))
    {
        if (ImGui::Selectable("(default)", ctx.gui.fontFamily.get() == "(default)"))
        {
            setDefaultFont(ctx, io);
        }

        int i = 0;
        for (const auto& font : ctx.gui.fonts)
        {
            ImGui::PushID(i++);
            if (ImGui::Selectable(font.family.c_str(), &font == ctx.gui.currentFont))
            {
                setFont(ctx, io, font, font.styles[0]);
            }
            ImGui::PopID();
        }
    }
    if (auto _ = ImGui::CreateCombo("Font style", ctx.gui.fontStyle.get().c_str()))
    {
        if (not ctx.gui.currentFont)
        {
            ImGui::Selectable("(default)", true);
        }
        else
        {
            int i = 0;
            for (const auto& style : ctx.gui.currentFont->styles)
            {
                ImGui::PushID(i++);
                if (ImGui::Selectable(style.name.c_str(), &style == ctx.gui.currentFontStyle))
                {
                    setFont(ctx, io, *ctx.gui.currentFont, style);
                }
                ImGui::PopID();
            }
        }
    }
    if (ImGui::InputFloat("Font size", &ctx.gui.fontSize.get(), 0.1f, 20.0f, "%.1f"))
    {
        auto& style = ImGui::GetStyle();
        style._NextFrameFontSizeBase = ctx.gui.fontSize;
    }
}

ALWAYS_INLINE static void drawConfigWindow(Context& ctx)
{
    if (not ctx.gui.configWindow)
    {
        return;
    }

    ImGui::OpenPopup("Configuration");

    auto& io = ImGui::GetIO();

    constexpr auto configWindowFlags
        = ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoMove;

    auto popup = ImGui::CreatePopupModal("Configuration", &ctx.gui.configWindow, configWindowFlags);

    if (not popup)
    {
        return;
    }

    if (auto _ = ImGui::CreateTabBar("Config"))
    {
        if (auto _ = ImGui::CreateTabItem("View"))
        {
            drawViewConfig(ctx, io);
        }
    }
}

static void drawMessage(Context& ctx, Message& msg, ImGuiIO& io, int i)
{
    const auto textSize = ImGui::CalcTextSize(msg.text.c_str());

    const ImVec2 pos(
        io.DisplaySize.x - textSize.x - 2 * textSize.y,
        io.DisplaySize.y - (3 + 3 * i) * textSize.y);

    ImVec4 color;

    switch (msg.severity)
    {
        case core::Severity::warning:
            color = ImGui::ColorFromHex(0xffff00);
            break;
        case core::Severity::error:
            color = ImGui::ColorFromHex(0xff0000);
            break;
        default:
            color = ImGui::ColorFromHex(0xffffff);
            break;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::SetNextWindowPos(pos, ImGuiCond_None, ImVec2(0.0, 0.0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0);
    ImGui::PushStyleVar(
        ImGuiStyleVar_Alpha,
        msg.time > ctx.gui.messageTime
        ? 1.0f - (msg.time - ctx.gui.messageTime) / float(ctx.gui.messageFadeOutTime)
        : 1.0f);

    constexpr auto messageWindowFlags
        = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoFocusOnAppearing;

    char windowName[32];
    snprintf(windowName, sizeof(windowName), "Msg%u", i);

    auto window = ImGui::CreateWindow(
        windowName,
        nullptr,
        messageWindowFlags);

    if (window)
    {
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::Text("%s", msg.text.c_str());
        if (ImGui::WasWindowClicked(ImGuiMouseButton_Left))
        {
            msg.time = ctx.gui.messageTime + ctx.gui.messageFadeOutTime;
        }
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

static void drawMessages(Context& ctx)
{
    auto& messages = ctx.gui.messages;
    auto& io = ImGui::GetIO();
    int i = 0;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.1, 0.1, 0.1, 0.8});

    for (auto it = messages.begin(); it != messages.end(); ++i)
    {
        auto& msg = *it;

        if (msg.time++ >= ctx.gui.messageTime + ctx.gui.messageFadeOutTime)
        {
            messages.erase(it++);
            continue;
        }

        drawMessage(ctx, msg, io, i);

        ++it;
    }

    ImGui::PopStyleColor();
}

void frame(unsigned int gameTextureId)
{
    const auto dockspaceId = ImGui::GetID("vgb dockspace");
    const auto viewport = ImGui::GetMainViewport();

    ImGui::DockSpaceOverViewport(dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    auto& ctx = *reinterpret_cast<Context*>(gb.debuggerData);

    drawMenuBar(ctx);
    drawLcd(ctx, gameTextureId);
    drawCartridgeWindow(ctx);
    drawCpuWindow(ctx);
    drawMemoryWindow(ctx);
    drawIoWindow(ctx);
    drawConsoleWindow(ctx);
    drawMapWindow(ctx);
    drawStyleEditorWindow(ctx);
    drawGameWindow(ctx);
    drawLogWindow(ctx);
    drawEmulationWindow(ctx);
    drawDisassemblyWindow(ctx);
    drawCallstackWindow(ctx);

    if (ctx.gui.demoWindow)
    {
        ImGui::ShowDemoWindow(&ctx.gui.demoWindow.get());
    }

    drawMessages(ctx);
    drawConfigWindow(ctx);

    if (ImGui::IsKeyReleased(ImGuiKey_GraveAccent))
    {
        if (ctx.gui.consoleWindow ^= true)
        {
            ImGui::SetWindowFocus("Console");
            ctx.gui.focusCmdLine = true;
        }
    }
}

}  // namespace debugger
