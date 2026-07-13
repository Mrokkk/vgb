#include "lexer.hpp"

#include <expected>
#include <map>
#include <stack>

#include <fmt/base.h>

#include "assembler/context.hpp"
#include "assembler/glue.hpp"
#include "assembler/helpers.hpp"
#include "assembler/parser.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"
#include "interpreter/lexer.hpp"
#include "interpreter/lexer_printers.hpp"
#include "sys/mapped_file.hpp"
#include "sys/platform.hpp"
#include "utils/immobile.hpp"

using namespace cpu::isa;
using namespace interpreter;

namespace assembler
{

struct Buffer final
{
    size_t           line;
    const char*      codeStart;
    const char*      codeEnd;
    const char*      lineStart;
    const char*      currentPos;
    Token*           current;
    Token*           previous;
    std::string      fileName;
    sys::MappedFile  mapped;
    Tokens           tokens;
};

using BufferStack = std::stack<Buffer>;

struct LexerContext final : utils::Immobile
{
    sys::MappedFile toUnmapLater;
    Buffer*         currentBuffer;
    BufferStack     buffers;
    Parser          parser;
};

static std::map<std::string_view, Operand::Type> operands{
#define OPERAND_MAPPING(NAME, OP) {#NAME, Operand::OP}
    OPERAND_MAPPING(a,  A),
    OPERAND_MAPPING(b,  B),
    OPERAND_MAPPING(c,  C),
    OPERAND_MAPPING(d,  D),
    OPERAND_MAPPING(e,  E),
    OPERAND_MAPPING(h,  H),
    OPERAND_MAPPING(l,  L),
    OPERAND_MAPPING(af, AF),
    OPERAND_MAPPING(bc, BC),
    OPERAND_MAPPING(de, DE),
    OPERAND_MAPPING(hl, HL),
    OPERAND_MAPPING(sp, SP),
    OPERAND_MAPPING(z,  FlagZ),
    OPERAND_MAPPING(nz, FlagNZ),
    OPERAND_MAPPING(nc, FlagNC),
};

static std::map<std::string_view, Opcode::Type> mnemoMapping{
#define MNEMO(UPPER, LOWER) {#LOWER, Opcode::UPPER},
#include "cpu/isa/mnemos.hpp"
#undef MNEMO
};

const std::map<std::string_view, SectionType> sections{
#define MAPPING(NAME) {#NAME, SectionType::NAME}
    MAPPING(ROM0),
#undef MAPPING
};

const std::map<std::string_view, Parser::token_type> tokens{
#define MAPPING(NAME, TOKEN) {#NAME, Parser::token_type::TOKEN}
    MAPPING(SECTION, SectionDirective),
    MAPPING(INCLUDE, IncludeDirective),
};

static bool debugEnabled = false;

static std::string_view mergeTokens(Token* start, Token* end)
{
    return std::string_view(start->value.begin(), end->value.end());
}

Parser::symbol_type parseNext(Context& mainContext)
{
    auto& ctx = *mainContext.lexerContext;

    Token* token;
    Token::Type tokenType = Token::Type::End;

    do
    {
        auto& buf = *ctx.currentBuffer;

        if (buf.previous and buf.previous->type == Token::Type::Newline)
        {
            buf.line++;
            buf.lineStart = buf.previous->value.end();
        }

        token = buf.previous = buf.current++;

        while (token->type == Token::Type::Whitespace or
            token->type == Token::Type::Comment)
        {
            token = buf.current++;
        }

        buf.currentPos = token->type == Token::Type::End
            ? buf.codeEnd
            : token->value.begin();

        tokenType = token->type;
        if (token->type == Token::Type::End)
        {
            ctx.toUnmapLater = std::move(buf.mapped);
            ctx.buffers.pop();
            if (ctx.buffers.empty())
            {
                ctx.currentBuffer = nullptr;
                return ctx.parser.make_YYEOF();
            }
            else
            {
                ctx.currentBuffer = &ctx.buffers.top();
            }
        }
    }
    while (tokenType == Token::Type::End);

    auto& buf = *ctx.currentBuffer;

    switch (tokenType)
    {
#define GENERIC_TOKEN(TOKEN, ...) \
    case Token::Type::TOKEN: \
        return ctx.parser.make_##TOKEN(__VA_ARGS__)

        GENERIC_TOKEN(Comma);
        GENERIC_TOKEN(Add);
        GENERIC_TOKEN(Sub);
        GENERIC_TOKEN(Mult);
        GENERIC_TOKEN(Div);
        GENERIC_TOKEN(LeftSquareBracket);
        GENERIC_TOKEN(RightSquareBracket);
        GENERIC_TOKEN(LeftParenthesis);
        GENERIC_TOKEN(RightParenthesis);
        GENERIC_TOKEN(Colon);
        GENERIC_TOKEN(Newline);
        GENERIC_TOKEN(StringLiteral, token->value);
        GENERIC_TOKEN(IntLiteral, token->value);

#undef GENERIC_TOKEN

        case Token::Type::Identifier:
            if (auto mnemo = findValue(mnemoMapping, token->value))
            {
                return ctx.parser.make_Instruction(*mnemo);
            }
            else if (auto operand = findValue(operands, token->value))
            {
                return ctx.parser.make_Operand(*operand);
            }
            else if (auto directive = findValue(tokens, token->value))
            {
                return Parser::symbol_type(*directive);
            }
            else if (auto section = findValue(sections, token->value))
            {
                return Parser::make_SectionType(*section);
            }
            else
            {
                return ctx.parser.make_Label(token->value);
            }

        case Token::Type::Dollar:
            if (buf.current->type == Token::Type::IntLiteral or buf.current->type == Token::Type::Identifier)
            {
                return ctx.parser.make_IntLiteral(mergeTokens(token, buf.current++));
            }
            else
            {
                return ctx.parser.make_Dollar();
            }

        case Token::Type::Percent:
            if (buf.current->type == Token::Type::IntLiteral or buf.current->type == Token::Type::Identifier)
            {
                return ctx.parser.make_IntLiteral(mergeTokens(token, buf.current++));
            }
            else
            {
                return ctx.parser.make_Mod();
            }

        default:
            reportError(mainContext, "unknown token: \"{}\"", token->value);
            return ctx.parser.make_YYerror();
    }
}

MaybeLocation getCurrentLocation(Context* mainContext)
{
    if (not mainContext or not mainContext->lexerContext)
    {
        return std::unexpected(false);
    }

    auto& ctx = *mainContext->lexerContext;

    if (not ctx.currentBuffer)
    {
        return std::unexpected(false);
    }

    auto& buf = *ctx.currentBuffer;

    auto token = buf.previous
        ? buf.previous
        : buf.current;

    while (token->type != Token::Type::Newline and
        token->type != Token::Type::End)
    {
        token++;
    }

    const char* lineEnd = token->type == interpreter::Token::Type::Newline
        ? token->value.begin()
        : buf.codeEnd;

    return Location{
        .lineNo = buf.line + 1,
        .pos = static_cast<size_t>(buf.currentPos - buf.lineStart) + 1,
        .fileName{buf.fileName},
        .line{buf.lineStart, lineEnd},
    };
}

static bool loadBuffer(Context& mainContext, LexerContext& ctx, const std::string_view& fileName)
{
    auto maybeMapped = sys::mapFile(fileName);

    if (not maybeMapped) [[unlikely]]
    {
        reportError(mainContext, "Failed to read \"{}\": {}", fileName, maybeMapped.error());
        return false;
    }

    std::string_view code(maybeMapped->getData<const char>(), maybeMapped->getSize());

    auto res = interpreter::parse(code, ';', true);

    if (not res)
    {
        reportError(mainContext, "{}", res.error());
        return false;
    }

    auto& currentBuffer = ctx.buffers.emplace(Buffer{
        .line = 0,
        .codeStart = code.begin(),
        .codeEnd = code.end(),
        .lineStart = code.begin(),
        .currentPos = code.begin(),
        .current = nullptr,
        .previous = nullptr,
        .fileName = std::string(fileName),
        .mapped = std::move(*maybeMapped),
        .tokens = std::move(*res),
    });

    ctx.currentBuffer = &currentBuffer;
    currentBuffer.current = currentBuffer.tokens.data();

    return true;
}

void parse(const std::string_view& fileName, Context& mainContext)
{
    LexerContext ctx{
        .parser{mainContext},
    };

    if (not loadBuffer(mainContext, ctx, fileName))
    {
        return;
    }

    mainContext.lexerContext = &ctx;

    ctx.parser.parse();

    mainContext.lexerContext = nullptr;
}

void handleIncludeDirective(Context& mainContext, const std::string_view& fileName)
{
    auto& ctx = *mainContext.lexerContext;
    loadBuffer(mainContext, ctx, fileName);
}

void setDebugging(bool enabled)
{
    debugEnabled = enabled;
}

}  // namespace assembler

assembler::Parser::symbol_type lex(assembler::Context& ctx)
{
    return assembler::parseNext(ctx);
}
