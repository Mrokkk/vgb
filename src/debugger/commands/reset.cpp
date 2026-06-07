#include "debugger/interpreter/command.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(reset)
{
    EXECUTOR()
    {
        if (not state.stopped)
        {
            logToConsole(state, "Cannot reset running emulation, press Ctrl+C to break");
            return 0;
        }
        gb.reset();
        return 0;
    }

    HELP() = "Reset emulation";
}

}  // namespace debugger::commands
