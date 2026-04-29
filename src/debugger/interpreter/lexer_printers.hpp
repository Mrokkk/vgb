#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "debugger/interpreter/lexer.hpp"

template <>
struct fmt::formatter<debugger::interpreter::Token> : fmt::formatter<string_view>
{
    format_context::iterator format(const debugger::interpreter::Token& token, format_context& ctx) const;
};

template <>
struct fmt::formatter<debugger::interpreter::Token::Type> : fmt::formatter<string_view>
{
    format_context::iterator format(debugger::interpreter::Token::Type type, format_context& ctx) const;
};
