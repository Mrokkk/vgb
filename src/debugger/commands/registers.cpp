#include "debugger/context.hpp"
#include "debugger/printer.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(registers)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        printCpuRegs(ctx);
        return 0;
    }

    HELP() = "Print CPU registers";
}

}  // namespace debugger::commands
