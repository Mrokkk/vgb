#include "debugger/context.hpp"
#include "game_boy.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(reset)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        ctx.gb.reset();
        return 0;
    }

    HELP() = "Reset emulation";
}

}  // namespace debugger::commands
