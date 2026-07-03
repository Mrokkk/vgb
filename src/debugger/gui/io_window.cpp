#include "io_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"

namespace debugger::gui
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

void drawIoWindow(Context& ctx)
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

}  // namespace debugger::gui
