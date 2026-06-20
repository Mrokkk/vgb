#define IMGUI_DEFINE_MATH_OPERATORS
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <strings.h>
#include <utility>

#include <fmt/fmt_ext.h>
#include <imgui.h>
#include <imgui_ext.h>
#include <imgui_internal.h>
#include <imgui_memory_editor/imgui_memory_editor.h>

#include "cpu/sm83.hpp"
#include "debugger/parser.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"
#include "logger.hpp"
#include "logger_reader.hpp"
#include "memory/memory_map.hpp"
#include "ppu.hpp"
#include "save_manager.hpp"
#include "utils/inline.hpp"
#include "utils/unique_ptr.hpp"
#include "utils/units.hpp"

namespace debugger
{

struct BitEntry
{
    uint8_t     shift;
    uint8_t     mask;
    const char* name;
};

struct IoEntry
{
    uint8_t     addr;
    const char* name;
    const char* desc;
    BitEntry    bitEntries[8];
};

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

static std::string buffer;
static std::vector<uint32_t> offsets;

static void onLog(const LogEntry& entry)
{
    char timeBuf[32];

    struct tm tmInfo;
    localtime_r(&entry.time, &tmInfo);

    strftime(timeBuf, sizeof(timeBuf), "%F %T", &tmInfo);

    offsets.push_back(buffer.size());

    if (entry.header)
    {
        buffer += (fmt::format_to_string("[{}] [{}] {}: {}\n",
            timeBuf,
            entry.header,
            entry.location.func,
            entry.message));
    }
    else
    {
        buffer += (fmt::format_to_string("[{}] {}: {}\n",
            timeBuf,
            entry.location.func,
            entry.message));
    }
}

void initImGui(State& state)
{
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();

    style.FontSizeBase      = 16;
    style.WindowBorderSize  = 0;
    style.FrameRounding     = 5.0f;
    style.ScrollbarRounding = 5.0f;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    state.gui.emulationWindow   = true;
    state.gui.cartridgeWindow   = true;
    state.gui.cpuWindow         = true;
    state.gui.consoleWindow     = true;
    state.gui.mapWindow         = false;
    state.gui.showScxScy        = true;
    state.gui.styleEditorWindow = false;
    state.gui.ioWindow          = true;
    state.gui.gameWindow        = true;
    state.gui.focusCmdLine      = false;
    state.gui.logWindow         = true;
    state.gui.lineBuffer[0]     = '\0';
    state.gui.addrBuffer[0]     = '\0';
    state.gui.ioFilterBuffer[0] = '\0';

    memEditor = utils::makeUnique<MemoryEditor>();
    memEditor->ReadFn          = &readMemory;
    memEditor->WriteFn         = &writeMemory;
    memEditor->Cols            = 8;
    memEditor->OptShowOptions  = false;
    memEditor->OptUpperCaseHex = false;
    memEditor->Open            = true;

    loggerReader.forEachLogEntry(
        [](const LogEntry& e)
        {
            onLog(e);
        });

    loggerReader.onLog(&onLog);
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

ALWAYS_INLINE static void drawMenuBar(State& state)
{
    auto mainMenu = ImGui::CreateMainMenuBar();

    if (not mainMenu)
    {
        return;
    }

    if (auto menu = ImGui::CreateMenu("File"))
    {
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
        BOOL_MENU_ITEM("Emulation", nullptr, state.gui.emulationWindow);
        BOOL_MENU_ITEM("Cartridge", nullptr, state.gui.cartridgeWindow);
        BOOL_MENU_ITEM("CPU", nullptr, state.gui.cpuWindow);
        BOOL_MENU_ITEM("Memory", nullptr, memEditor->Open);
        BOOL_MENU_ITEM("IO", nullptr, state.gui.ioWindow);
        BOOL_MENU_ITEM("Map", nullptr, state.gui.mapWindow);
        BOOL_MENU_ITEM("Console", "`", state.gui.consoleWindow);
        BOOL_MENU_ITEM("Game", nullptr, state.gui.gameWindow);
        BOOL_MENU_ITEM("Log", nullptr, state.gui.logWindow);
        BOOL_MENU_ITEM("Style Editor", nullptr, state.gui.styleEditorWindow);
        BOOL_MENU_ITEM("Demo", nullptr, state.gui.demoWindow);
    }
}

ALWAYS_INLINE static void drawLcd(State&, unsigned int gameTextureId, int fps)
{
    auto window = ImGui::CreateWindow("LCD");

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text(ImGui::IsWindowFocused() ? "%d FPS; speed: %ux" : "%d FPS; speed: %ux  (click to focus)", fps, gb.speedMultiplier);
    const auto viewportSize = ImGui::GetContentRegionAvail();
    const auto imageSize = scaleToRatio(viewportSize, GB_LCD_RESX, GB_LCD_RESY);
    ImGui::SetCursorPos((viewportSize - imageSize) * 0.5 + ImGui::GetCursorPos());
    ImGui::Image(static_cast<ImTextureID>(gameTextureId), imageSize);
    gb.inputEnabled = ImGui::IsWindowFocused();
}

ALWAYS_INLINE static void drawCartridgeWindow(State& state)
{
    if (not state.gui.cartridgeWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("Cartridge", &state.gui.cartridgeWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text("Path: %s", gb.config.cartridgePath.c_str());
    ImGui::Text("Title: %s", gb.cartridge.getTitle());
    ImGui::Text("Type: %s", gb.cartridge.getType());

    auto romSize = utils::humanReadable(gb.cartridge.romSize());
    auto ramSize = utils::humanReadable(gb.cartridge.ramSize());

    ImGui::Text("ROM size: %zu %s", romSize.value, romSize.unit);
    ImGui::Text("RAM size: %zu %s", ramSize.value, ramSize.unit);
}

ALWAYS_INLINE static void drawCpuWindow(State& state)
{
    if (not state.gui.cpuWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("CPU", &state.gui.cpuWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

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
        ImGui::Text("Instructions: %zu", gb.cpu.instructions);
        ImGui::Text("T-cycles: %zu", gb.cpu.cycles);
    }
}

ALWAYS_INLINE static void drawMemoryWindow(State& state)
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

    if (ImGui::InputText("Go to address", state.gui.addrBuffer, sizeof(state.gui.addrBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        char* tmp;
        auto addr = strtoul(state.gui.addrBuffer, &tmp, 16);
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

ALWAYS_INLINE static void drawConsoleWindow(State& state)
{
    if (not state.gui.consoleWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("Console", &state.gui.consoleWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

    auto scroll = ImGui::CreateChild("##Scroll");

    if (not scroll)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0, 1});

    for (const auto& line : state.consoleLines)
    {
        ImGui::TextUnformatted(line.c_str());
    }

    ImGui::TextUnformatted("(vgb) ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});
    ImGui::PushItemWidth(-1);

    if (state.gui.commandEntered)
    {
        ImGui::SetScrollFromPosY(1.0f);
        state.gui.commandEntered = false;
    }

    if (ImGui::InputText("#CmdLine", state.gui.lineBuffer, sizeof(state.gui.lineBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        state.gui.commandEntered = true;

        std::string command(state.gui.lineBuffer[0] ? state.gui.lineBuffer : state.prevLine);
        state.gui.lineBuffer[0] = 0;

        logToConsole(state, "{} {}", state.prompt, command);

        auto parsed = parseCommand(command);

        if (not parsed) [[unlikely]]
        {
            state.consoleLines.push_back(std::move(parsed.error()));
        }
        else
        {
            parsed->command.handler(state, parsed->args);
            state.prevLine = std::move(command);
        }
        state.gui.focusCmdLine = true;
    }

    if (state.gui.focusCmdLine)
    {
        ImGui::SetKeyboardFocusHere(-1);
        state.gui.focusCmdLine = false;
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    state.gui.focusCmdLine = ImGui::IsMouseClicked(ImGuiMouseButton_Left) and ImGui::IsWindowHovered();

    if (ImGui::IsWindowFocused() and ImGui::IsKeyReleased(ImGuiKey_C) and ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        logToConsole(state, "Interrupted");
        gb.stop();
    }
}

ALWAYS_INLINE static void drawMapWindow(State& state)
{
    if (not state.gui.mapWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("Map", &state.gui.mapWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Checkbox("Show SCX/SCY window", &state.gui.showScxScy);
    const auto viewportSize = ImGui::GetContentRegionAvail();
    const auto imageSize = scaleToRatio(viewportSize, 1, 1);
    ImGui::SetCursorPos((viewportSize - imageSize) * 0.5 + ImGui::GetCursorPos());
    ImGui::Image(static_cast<ImTextureID>(gb.renderer->renderMap(state.gui.showScxScy)), imageSize);
}

ALWAYS_INLINE static void drawStyleEditorWindow(State& state)
{
    if (not state.gui.styleEditorWindow)
    {
        return;
    }

    if (auto window = ImGui::CreateWindow("Style Editor", &state.gui.styleEditorWindow))
    {
        ImGui::ShowStyleEditor();
    }
}

ALWAYS_INLINE static void drawIoWindow(State& state)
{
    if (not state.gui.ioWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("IO", &state.gui.ioWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::InputText("Filter", state.gui.ioFilterBuffer, sizeof(state.gui.ioFilterBuffer));

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

        const auto ioFilterLen = strlen(state.gui.ioFilterBuffer);

        for (const auto& entry : ioEntries)
        {
            if (state.gui.ioFilterBuffer[0] and strncasecmp(entry.name, state.gui.ioFilterBuffer, ioFilterLen))
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

ALWAYS_INLINE static void drawGameWindow(State& state)
{
    if (not state.gui.gameWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("Game", &state.gui.gameWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

    if (not state.game)
    {
        ImGui::TextDisabled("Not supported game");
        return;
    }

    state.game->drawUi();
}

ALWAYS_INLINE static void drawLogWindow(State& state)
{
    if (not state.gui.logWindow)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});

    if (auto window = ImGui::CreateWindow("Log", &state.gui.logWindow)) [[likely]]
    {
        ImGui::InputTextMultiline("##Log", buffer.data(), buffer.size() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

ALWAYS_INLINE static void drawEmulationWindow(State& state)
{
    if (ImGui::IsKeyReleased(ImGuiKey_F5))
    {
        SaveManager::quickSave();
    }
    else if (ImGui::IsKeyReleased(ImGuiKey_F9))
    {
        SaveManager::quickLoad();
    }

    if (not state.gui.emulationWindow)
    {
        return;
    }

    auto window = ImGui::CreateWindow("Emulation", &state.gui.emulationWindow);

    if (not window) [[unlikely]]
    {
        return;
    }

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
            gb.reset();
        }

        ImGui::SliderInt("Speed", reinterpret_cast<int*>(&gb.speedMultiplier), 1, 20);
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

void frame(unsigned int gameTextureId, int fps)
{
    const auto dockspaceId = ImGui::GetID("vgb dockspace");
    const auto viewport = ImGui::GetMainViewport();

    ImGui::DockSpaceOverViewport(dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    auto& state = *reinterpret_cast<State*>(gb.debuggerData);

    drawMenuBar(state);
    drawLcd(state, gameTextureId, fps);
    drawCartridgeWindow(state);
    drawCpuWindow(state);
    drawMemoryWindow(state);
    drawIoWindow(state);
    drawConsoleWindow(state);
    drawMapWindow(state);
    drawStyleEditorWindow(state);
    drawGameWindow(state);
    drawLogWindow(state);
    drawEmulationWindow(state);

    if (state.gui.demoWindow)
    {
        ImGui::ShowDemoWindow(&state.gui.demoWindow);
    }

    if (ImGui::IsKeyReleased(ImGuiKey_GraveAccent))
    {
        if (state.gui.consoleWindow ^= true)
        {
            state.gui.focusCmdLine = true;
        }
    }
}

}  // namespace debugger
