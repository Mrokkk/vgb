#include "debugger/interpreter/command.hpp"
#include "debugger/state.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(set)
{
    EXECUTOR()
    {
        auto name = ARGUMENT_GET(0, String);

        if (name == "printRegs")
        {
            state.printRegs = ARGUMENT_GET(1, Boolean);
        }

        return 0;
    }

    HELP() = "Set variable";
}

}  // namespace debugger::commands
