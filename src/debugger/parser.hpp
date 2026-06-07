#pragma once

#include <expected>

#include "debugger/interpreter/command.hpp"

namespace debugger
{

struct CommandData
{
    const interpreter::Command& command;
    interpreter::Arguments      args;
};

std::expected<CommandData, std::string> parseCommand(const std::string_view& line);

}  // namespace debugger
