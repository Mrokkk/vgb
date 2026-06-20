#include "debugger/interpreter/command.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(reset)
{
    EXECUTOR()
    {
        gb.reset();
        return 0;
    }

    HELP() = "Reset emulation";
}

}  // namespace debugger::commands
