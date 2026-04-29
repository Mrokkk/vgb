#include "command.hpp"

#include "debugger/interpreter/commands.hpp"

namespace debugger::interpreter
{

void CommandBase::$register(Command command)
{
    commands.$register(std::move(command));
}

}  // namespace debugger::interpreter
