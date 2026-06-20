#include "debugger/interpreter/command.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(continue)
{
    EXECUTOR()
    {
        gb.start();
        return 0;
    }

    HELP() = "Continue execution";
}

}  // namespace debugger::commands
