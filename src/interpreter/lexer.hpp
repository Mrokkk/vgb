#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace interpreter
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

std::expected<Tokens, std::string> parse(const std::string_view& code);

}  // namespace interpreter
