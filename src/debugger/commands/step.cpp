#include "debugger/printer.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(step)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        ctx.gb.cpu.step();
        printInstruction(ctx);
        return 0;
    }

    HELP() = "Step one instruction";
}

}  // namespace debugger::commands
