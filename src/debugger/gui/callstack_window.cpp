#include "callstack_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "debugger/symbols_map.hpp"

namespace debugger::gui
{

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

void drawCallstackWindow(Context& ctx)
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

}  // namespace debugger::gui
