#include "lexer_printers.hpp"

#include <fmt/base.h>

#include "interpreter/lexer.hpp"

using namespace interpreter;

fmt::format_context::iterator fmt::formatter<Token::Type>::format(Token::Type type, format_context& ctx) const
{
#define TOKEN_TYPE_PRINT(type) \
    case Token::Type::type: \
        return format_to(ctx.out(), #type)
    switch (type)
    {
        TOKEN_TYPE_PRINT(Comment);
        TOKEN_TYPE_PRINT(StringLiteral);
        TOKEN_TYPE_PRINT(IntLiteral);
        TOKEN_TYPE_PRINT(BooleanLiteral);
        TOKEN_TYPE_PRINT(Exclamation);
        TOKEN_TYPE_PRINT(Slash);
        TOKEN_TYPE_PRINT(Pipe);
        TOKEN_TYPE_PRINT(Dot);
        TOKEN_TYPE_PRINT(Add);
        TOKEN_TYPE_PRINT(Sub);
        TOKEN_TYPE_PRINT(Dollar);
        TOKEN_TYPE_PRINT(LeftParenthesis);
        TOKEN_TYPE_PRINT(RightParenthesis);
        TOKEN_TYPE_PRINT(LeftBracket);
        TOKEN_TYPE_PRINT(RightBracket);
        TOKEN_TYPE_PRINT(Percent);
        TOKEN_TYPE_PRINT(Identifier);
        TOKEN_TYPE_PRINT(Semicolon);
        TOKEN_TYPE_PRINT(Newline);
        TOKEN_TYPE_PRINT(Whitespace);
        TOKEN_TYPE_PRINT(End);
    }
    return format_to(ctx.out(), "unknown{{{}}}", static_cast<int>(type));
}

fmt::format_context::iterator fmt::formatter<Token>::format(const Token& token, format_context& ctx) const
{
    return format_to(ctx.out(), "{}{{{}}}", token.type, token.value);
}
