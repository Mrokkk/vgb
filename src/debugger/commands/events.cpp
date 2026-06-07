#include <fmt/base.h>

#include "debugger/interpreter/command.hpp"
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
            [&i](const Event& ev)
            {
                fmt::println("{}: {} in {} cycles", i++, ev.getName(), ev.getWhen() - gb.cpu.cycles);
            });
        return 0;
    }

    HELP() = "List scheduled events";
}

}  // namespace debugger::commands
