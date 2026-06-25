#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "interpreter/lexer.hpp"

template <>
struct fmt::formatter<interpreter::Token> : fmt::formatter<string_view>
{
    format_context::iterator format(const interpreter::Token& token, format_context& ctx) const;
};

template <>
struct fmt::formatter<interpreter::Token::Type> : fmt::formatter<string_view>
{
    format_context::iterator format(interpreter::Token::Type type, format_context& ctx) const;
};
