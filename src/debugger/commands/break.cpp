#include <fmt/base.h>
#include <fmt/fmt_ext.h>

#include "debugger/interpreter/command.hpp"
#include "debugger/state.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(break)
{
    EXECUTOR()
    {
        static uint32_t id = 1;
        auto address = ARGUMENT_GET(0, Integer);

        if (address > 0xffff)
        {
            logToConsole(state, "Invalid address: {:x}", address);
            return 1;
        }

        auto newId = id++;

        state.breakpoints[address] = Breakpoint{.address = (uint16_t)address, .id = newId};

        logToConsole(state, "Breakpoint {} at {:#04x}", newId, address);

        return 0;
    }

    HELP() = "Set a breakpoint";
}

}  // namespace debugger::commands
