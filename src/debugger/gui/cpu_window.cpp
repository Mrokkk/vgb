#include "cpu_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"

namespace debugger::gui
{

void drawCpuWindow(Context& ctx)
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
        constexpr size_t updateInterval = 60;
        auto deltaTime = ImGui::GetIO().DeltaTime;
        ONCE_PER_X_FRAMES(updateInterval)
        {
            ctx.gui.ips = ctx.gui.sumIps / updateInterval;
            ctx.gui.mhz = ctx.gui.sumMhz / updateInterval;
            ctx.gui.sumIps = 0;
            ctx.gui.sumMhz = 0;
        }

        if (gb.cpu.instructions > ctx.gui.prevInstructions) [[likely]]
        {
            ctx.gui.sumIps += (gb.cpu.instructions - ctx.gui.prevInstructions) / deltaTime;
            ctx.gui.sumMhz += (gb.cpu.cycles - ctx.gui.prevCycles) / deltaTime;
        }

        ctx.gui.prevInstructions = gb.cpu.instructions;
        ctx.gui.prevCycles = gb.cpu.cycles;

        ImGui::Text("Instructions: %zu", gb.cpu.instructions);
        ImGui::SameLineText("(%.02f kIPS)", ctx.gui.ips / 1000);
        ImGui::Text("T-cycles: %zu", gb.cpu.cycles);
        ImGui::SameLineText("(%.03f MHz)", ctx.gui.mhz / 1000000);
    }
}

}  // namespace debugger::gui
