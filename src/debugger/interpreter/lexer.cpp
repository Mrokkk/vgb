#include "lexer.hpp"

#include <cctype>
#include <expected>
#include <functional>
#include <string_view>
#include <vector>

#include <fmt/fmt_ext.h>

namespace debugger::interpreter
{

struct LexerState
{
    Tokens      tokens;
    const char* current;
    std::string error;
};

using TokenHandler = std::function<bool(LexerState&)>;

struct TokenMapping
{
    Token::Type         type;
    TokenHandler        handler;
};

using TokenHandlers = std::vector<TokenHandler>;

static char peek(LexerState& state)
{
    return *state.current;
}

static char peekNext(LexerState& state)
{
    return *(state.current + 1);
}

static void advance(LexerState& state)
{
    ++state.current;
}

static const char* current(LexerState& state)
{
    return state.current;
}

static bool isSpace(char c)
{
    return c == ' ' or c == '\t';
}

static std::string_view peekWord(LexerState& state)
{
    auto it = current(state);
    while (*it)
    {
        if (isSpace(*it) or *it == '\n')
        {
            break;
        }
        ++it;
    }
    return std::string_view(current(state), it);
}

static void advance(std::string_view view, LexerState& state)
{
    state.current = view.data() + view.size();
}

static void addToken(Token::Type type, const char* start, const char* end, LexerState& state)
{
    state.tokens.push_back(
        Token{
            .type = type,
            .value = std::string_view(start, end)
        });
}

static void addToken(Token::Type type, const char* start, LexerState& state)
{
    state.tokens.push_back(
        Token{
            .type = type,
            .value = std::string_view(start, current(state))
        });
}

static bool stringLiteral(LexerState& state)
{
    if (peek(state) != '\"')
    {
        return false;
    }

    advance(state);

    const auto start = current(state);

    bool foundEnd{false};

    while (const auto c = peek(state))
    {
        if (c == '\"')
        {
            advance(state);
            foundEnd = true;
            break;
        }
        else if (c == '\\' and peekNext(state) == '\"')
        {
            advance(state);
        }
        advance(state);
    }

    // FIXME: return some error
    if (not foundEnd)
    {
        return false;
    }

    addToken(Token::Type::StringLiteral, start, current(state) - 1, state);

    return true;
}

static bool intLiteral(LexerState& state)
{
    auto start = current(state);

    const auto c = peek(state);

    if (std::isdigit(c))
    {
        advance(state);
    }
    else
    {
        return false;
    }

    if (c == '0' and peek(state) == 'x')
    {
        advance(state);

        while (const auto c = peek(state))
        {
            if (not isxdigit(c))
            {
                break;
            }
            advance(state);
        }
    }
    else
    {
        while (const auto c = peek(state))
        {
            if (not isdigit(c))
            {
                break;
            }
            advance(state);
        }
    }

    addToken(Token::Type::IntLiteral, start, state);

    return true;
}

static bool comment(LexerState& state)
{
    const auto start = current(state);

    if (peek(state) != '#')
    {
        return false;
    }

    advance(state);

    while (const auto c = peek(state))
    {
        if (c == '\n')
        {
            break;
        }
        advance(state);
    }

    addToken(Token::Type::Comment, start, state);

    return true;
}

static bool identifier(LexerState& state)
{
    if (not std::isalpha(peek(state)))
    {
        return false;
    }

    const auto start = current(state);

    advance(state);

    while (const auto c = peek(state))
    {
        if (not std::isalnum(c))
        {
            break;
        }
        advance(state);
    }

    addToken(Token::Type::Identifier, start, state);

    return true;
}

static TokenHandler singleChar(const char character, Token::Type type)
{
    return
        [character, type](LexerState& state)
        {
            const auto start = current(state);
            if (peek(state) != character)
            {
                return false;
            }
            advance(state);
            addToken(type, start, state);
            return true;
        };
}

static bool space(LexerState& state)
{
    const auto c = peek(state);

    if (not isSpace(c))
    {
        return false;
    }

    const auto start = current(state);

    while (const auto c = peek(state))
    {
        if (not isSpace(c))
        {
            break;
        }
        advance(state);
    }

    addToken(Token::Type::Whitespace, start, state);

    return true;
}

static bool booleanLiteral(LexerState& state)
{
    auto word = peekWord(state);
    if (word == "true" or word == "false")
    {
        advance(word, state);
        addToken(Token::Type::BooleanLiteral, word.data(), current(state), state);
        return true;
    }
    return false;
}

static const TokenHandlers handlers = {
    space,
    comment,
    singleChar('\n', Token::Type::Newline),
    singleChar(';', Token::Type::Semicolon),
    singleChar('%', Token::Type::Percent),
    singleChar('!', Token::Type::Exclamation),
    singleChar('/', Token::Type::Slash),
    singleChar('|', Token::Type::Pipe),
    singleChar('.', Token::Type::Dot),
    singleChar('+', Token::Type::Add),
    singleChar('-', Token::Type::Sub),
    singleChar('(', Token::Type::LeftParenthesis),
    singleChar(')', Token::Type::RightParenthesis),
    singleChar('{', Token::Type::LeftBracket),
    singleChar('}', Token::Type::RightBracket),
    singleChar('$', Token::Type::Dollar),
    intLiteral,
    stringLiteral,
    booleanLiteral,
    identifier,
};

std::expected<Tokens, std::string> parse(const std::string_view& code)
{
    LexerState state{.current = code.data()};

    while (peek(state))
    {
        bool found{false};

        for (const auto& handler : handlers)
        {
            if (handler(state))
            {
                found = true;
                break;
            }
        }

        if (not found) [[unlikely]]
        {
            state.error = fmt::format_to_string("unknown token at \"{}\"", state.current);
            return std::unexpected(std::move(state.error));
        }
    }

    state.tokens.emplace_back(Token{.type = Token::Type::End});

    return std::move(state.tokens);
}

}  // namespace debugger::interpreter
