#include "main.hpp"

#include <map>
#include <unistd.h>

#include "cpu/exception.hpp"
#include "cpu/printers.hpp"
#include "cpu/sm83.hpp"
#include "debugger/games/registry.hpp"
#include "debugger/imgui.hpp"
#include "debugger/printer.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"

namespace debugger
{

static void runCpu(State& state)
{
    while (1)
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            break;
        }
        if (static_cast<int>(state.cpu.pc) != state.prevBreakpoint)
        {
            const auto breakpoint = state.breakpoints.find(state.cpu.pc);

            if (breakpoint != state.breakpoints.end())
            {
                logToConsole(state, "Hit breakpoint {} at {:#04x}", breakpoint->second.id, breakpoint->second.address);
                state.prevBreakpoint = state.cpu.pc;
                gb.stop();
                break;
            }
            else
            {
                state.prevBreakpoint = -1;
            }
        }

        if (state.cpu.step())
        {
            gb.stop();
            break;
        }
    }
    if (state.cpu.exc)
    {
        logToConsole(state, "Exception raised: {}", state.cpu.exc);
    }
    printInstruction(state, state.cpu);
}

void main(GameBoy& gb)
{
    State state{
        .cpu = gb.cpu,
        .prevBreakpoint = -1,
        .prompt = "(vgb)",
    };

    initImGui(state);
    logToConsole(state, "Debugger mode");
    logToConsole(state, "For help, type \"help\"");

    gb.debuggerData = static_cast<void*>(&state);

    state.game = games::Registry::createGame(gb.cartridge.getTitle());

    while (1)
    {
        if (state.cpu.exc.type == cpu::Exception::UserInterruption)
        {
            break;
        }

        while (gb.state == GameBoy::State::Stopped)
        {
            gb.frame();

            if (state.cpu.exc.type == cpu::Exception::UserInterruption)
            {
                goto finish;
            }
        }

        runCpu(state);
    }

finish:
    gb.debuggerData = nullptr;
}

}  // namespace debugger
