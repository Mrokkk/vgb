#pragma once

#include <string>

#include "interpreter/command.hpp"
#include "interpreter/operations.hpp"
#include "utils/function_ref.hpp"

namespace interpreter
{

void initialize(Operations ops, void* defaultUserData);
void setOperations();
void setAlias(std::string alias, std::string command);
void forEachAlias(utils::FunctionRef<void(const std::string&, const std::string&)> callback);
void registerCommand(Command command);
void forEachCommand(utils::FunctionRef<void(const Command&)> callback);

ExecutionResult exectuteCommand(std::string command);

}  // namespace interpreter
