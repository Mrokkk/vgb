#include <fmt/base.h>

#include "debugger/interpreter/command.hpp"
#include "debugger/interpreter/commands.hpp"

namespace debugger::commands
{

DEFINE_COMMAND(help)
{
    EXECUTOR()
    {
        interpreter::commands.forEach(
            [](const interpreter::Command& command)
            {
                fmt::println("{:<30} {}", command.name, command.help);
            });
        return 0;
    }

    HELP() = "Print help";
}

}  // namespace debugger::commands
