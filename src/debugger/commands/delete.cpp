#include <fmt/format.h>

#include "debugger/context.hpp"
#include "interpreter/command.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(delete)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        auto id = GET_ARGUMENT(0, Integer);

        if (id < 0)
        {
            return std::unexpected(fmt::format("Invalid breakpoint number: {}", id));
        }

        for (auto it = ctx.breakpoints.begin(); it != ctx.breakpoints.end(); ++it)
        {
            if (it->second.id == id)
            {
                ctx.breakpoints.erase(it);
                break;
            }
        }
        return 0;
    }

    HELP() = "Delete a breakpoint";
}

}  // namespace debugger::commands
