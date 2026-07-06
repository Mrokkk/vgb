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
        Comma,
        Add,
        Sub,
        Dollar,
        LeftParenthesis,
        RightParenthesis,
        LeftBracket,
        RightBracket,
        LeftSquareBracket,
        RightSquareBracket,
        Percent,
        Identifier,
        Colon,
        Semicolon,
        Newline,
        Whitespace,
        End,
    };

    Type             type;
    std::string_view value;
};

using Tokens = std::vector<Token>;
using MaybeTokens = std::expected<Tokens, std::string>;

MaybeTokens parse(
    const std::string_view& code,
    char commentChar = '#',
    bool allIntegersAsHex = false);

}  // namespace interpreter
