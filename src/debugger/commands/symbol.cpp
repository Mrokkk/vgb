#include "debugger/context.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(symbol)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        auto symbolName = GET_ARGUMENT(0, String);
        auto symbol = ctx.symbols[symbolName];

        if (not symbol) [[unlikely]]
        {
            ctx.console.addLine("No such symbol");
            return 1;
        }

        ctx.console.writeLine("{} at {:04x}-{:04x} (size {:x}) bank {:02x}",
            symbol->name,
            symbol->start,
            symbol->start + symbol->size,
            symbol->size,
            symbol->bank);

        return 0;
    }

    HELP() = "Get info about symbol";
}

}  // namespace debugger::commands
