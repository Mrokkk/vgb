#include <fmt/base.h>

#include "debugger/context.hpp"
#include "interpreter/command.hpp"
#include "interpreter/interpreter.hpp"

namespace debugger::commands
{

DEFINE_AND_REGISTER_COMMAND(help)
{
    EXECUTOR()
    {
        auto& ctx = GET_USER_DATA(Context);
        interpreter::forEachCommand(
            [&ctx](const interpreter::Command& command)
            {
                ctx.console.writeLine("{:<30} {}", command.name, command.help);
            });
        return 0;
    }

    HELP() = "Print help";
}

}  // namespace debugger::commands
