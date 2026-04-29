#include "debugger/interpreter/command.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(continue)
{
    EXECUTOR()
    {
        state.stopped = false;
        gb.cpu.stopped = false;
        return 0;
    }

    HELP() = "Continue execution";
}

}  // namespace debugger::commands
