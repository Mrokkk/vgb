#include <fmt/format.h>

#include "debugger/context.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(break)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);

        static uint32_t id = 1;
        long address = 0;

        if (ARGC() == 0)
        {
            for (const auto& [_, breakpoint] : ctx.breakpoints)
            {
                ctx.console.writeLine("Breakpoint {} at {:#04x}", breakpoint.id, breakpoint.address);
            }
            return 0;
        }

        if (args[0].isString())
        {
            const auto& symbolName = *args[0].getString();
            auto symbol = ctx.symbols[symbolName];
            if (not symbol)
            {
                return std::unexpected(fmt::format("No such symbol: {}", symbolName));
            }
            address = symbol->start;
        }
        else
        {
            address = GET_ARGUMENT(0, Integer);
        }

        if (address < 0 or address > 0xffff)
        {
            return std::unexpected(fmt::format("Invalid address: {:x}", address));
        }

        auto newId = id++;

        ctx.breakpoints[address] = Breakpoint{.address = (uint16_t)address, .id = newId};

        ctx.console.writeLine("Breakpoint {} at {:#04x}", newId, address);

        return 0;
    }

    HELP() = "Set a breakpoint";
}

}  // namespace debugger::commands
