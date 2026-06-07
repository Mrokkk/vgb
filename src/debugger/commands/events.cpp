#include <fmt/base.h>

#include "debugger/interpreter/command.hpp"
#include "debugger/state.hpp"
#include "event.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(events)
{
    EXECUTOR()
    {
        int i = 0;
        gb.events.forEachEvent(
            [&i, &state](const Event& ev)
            {
                logToConsole(state, "{}: {} in {} cycles", i++, ev.getName(), ev.getWhen() - gb.cpu.cycles);
            });
        return 0;
    }

    HELP() = "List scheduled events";
}

}  // namespace debugger::commands
