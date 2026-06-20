#include "debugger/interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(set)
{
    EXECUTOR()
    {
        return 0;
    }

    HELP() = "Set variable";
}

}  // namespace debugger::commands
