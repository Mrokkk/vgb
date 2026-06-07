#include <fmt/base.h>

#include "debugger/interpreter/command.hpp"
#include "debugger/interpreter/commands.hpp"
#include "debugger/state.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(help)
{
    EXECUTOR()
    {
        interpreter::commands.forEach(
            [&state](const interpreter::Command& command)
            {
                logToConsole(state, "{:<30} {}", command.name, command.help);
            });
        return 0;
    }

    HELP() = "Print help";
}

}  // namespace debugger::commands
