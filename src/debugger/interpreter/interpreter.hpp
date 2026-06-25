#pragma once

#include <cstddef>
#include <expected>
#include <string>

#include "debugger/state.hpp"

namespace debugger::interpreter
{

std::expected<std::nullptr_t, std::string> exectuteCommand(std::string command, State& state);

}  // namespace debugger::interpreter
