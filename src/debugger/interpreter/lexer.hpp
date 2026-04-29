#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "utils/fwd.hpp"

namespace debugger::interpreter
{

struct Token
{
    enum class Type
    {
        Comment,
        StringLiteral,
        IntLiteral,
        BooleanLiteral,
        Exclamation,
        Slash,
        Pipe,
        Dot,
        Add,
        Sub,
        Dollar,
        LeftParenthesis,
        RightParenthesis,
        LeftBracket,
        RightBracket,
        Percent,
        Identifier,
        Semicolon,
        Newline,
        Whitespace,
        End,
    };

    Type             type;
    std::string_view value;
};

using Tokens = std::vector<Token>;

std::expected<Tokens, std::string> parse(const std::string& code);

utils::Buffer& operator<<(utils::Buffer& buf, const Token::Type type);
utils::Buffer& operator<<(utils::Buffer& buf, const Token& token);

}  // namespace debugger::interpreter
