#pragma once

#include <string_view>

#include "debugger/interpreter/command.hpp"
#include "utils/function_ref.hpp"

namespace debugger::interpreter
{

struct CommandBase;

struct Commands final
{
    static Command* find(const std::string_view& name);
    static void forEach(utils::FunctionRef<void(const Command&)> callback);

private:
    friend CommandBase;
    static void $register(Command command);
};

constexpr static Commands commands;

}  // namespace debugger::interpreter
