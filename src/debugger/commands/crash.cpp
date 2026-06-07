#include "debugger/interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(crash)
{
    EXECUTOR()
    {
        *((int*)0) = 2137;
        return 0;
    }

    HELP() = "Crash application";
}

}  // namespace debugger::commands
