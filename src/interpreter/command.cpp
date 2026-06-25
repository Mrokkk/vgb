#include "command.hpp"

#include "interpreter/interpreter.hpp"

namespace interpreter
{

void CommandBase::$register(Command command)
{
    registerCommand(std::move(command));
}

}  // namespace interpreter
