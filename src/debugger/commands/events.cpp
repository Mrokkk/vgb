#include <fmt/base.h>

#include "debugger/context.hpp"
#include "event.hpp"
#include "game_boy.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(events)
{
    EXECUTOR()
    {
        int i = 0;
        auto& ctx = GET_USER_DATA(Context);

        gb.events.forEachEvent(
            [&i, &ctx](const Event& ev)
            {
                ctx.console.writeLine("{}: {} in {} cycles", i++, ev.getName(), ev.getWhen() - gb.cpu.cycles);
            });
        return 0;
    }

    HELP() = "List scheduled events";
}

}  // namespace debugger::commands
