#include <fmt/base.h>
#include <fmt/fmt_ext.h>

#include "debugger/context.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(break)
{
    EXECUTOR()
    {
        static uint32_t id = 1;
        auto address = GET_ARGUMENT(0, Integer);

        auto& ctx = GET_USER_DATA(Context);

        if (address < 0 or address > 0xffff)
        {
            ctx.console.writeLine("Invalid address: {:x}", address);
            return 1;
        }

        auto newId = id++;

        ctx.breakpoints[address] = Breakpoint{.address = (uint16_t)address, .id = newId};

        ctx.console.writeLine("Breakpoint {} at {:#04x}", newId, address);

        return 0;
    }

    HELP() = "Set a breakpoint";
}

}  // namespace debugger::commands
