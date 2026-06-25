#include "debugger/context.hpp"
#include "interpreter/command.hpp"
#include "game_boy.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(continue)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        ctx.gb.start();
        return 0;
    }

    HELP() = "Continue execution";
}

}  // namespace debugger::commands
