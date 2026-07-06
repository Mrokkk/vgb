#include "assembler.hpp"

#include <charconv>
#include <cstdint>
#include <expected>
#include <map>

#include <fmt/base.h>

#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"
#include "cpu/isa/printers.hpp"
#include "error.hpp"
#include "game_boy.hpp"
#include "interpreter/lexer.hpp"
#include "interpreter/lexer_printers.hpp"
#include "memory/cartridge.hpp"
#include "sys/platform.hpp"
#include "utils/byte_order.hpp"
#include "utils/maybe.hpp"
#include "utils/units.hpp"

using utils::Maybe;
using interpreter::Token;
using interpreter::Tokens;
using namespace utils::literals;

namespace cpu::isa
{

namespace
{

using NameToOperandType = std::map<std::string_view, Operand::Type>;
using NameToOpcodeType = std::map<std::string_view, Opcode::Type>;
using OpcodeTypeToOpcodes = std::map<Opcode::Type, std::vector<const Opcode*>>;

static NameToOpcodeType    opcodeType;
static NameToOperandType   registers;
static NameToOperandType   flags;
static OpcodeTypeToOpcodes mapping;

struct LabelOffset final
{
    bool        relative;
    size_t      offset;
    std::string label;
};

using LabelToAddress = std::map<std::string, uint32_t>;
using Data = std::vector<uint8_t>;
using LabelOffsets = std::vector<LabelOffset>;

enum SectionType
{
    ROM0 = 0x0000,
};

struct Section
{
    SectionType type;
    size_t      currentOffset;
};

struct UserSection final
{
    Section*    section;
    size_t      currentOffset;
};

using UserSections = std::map<std::string_view, UserSection>;

struct Context final
{
    size_t         tokenIndex;
    const Tokens   tokens;
    const Opcode*  opcodes;
    size_t         dataOffset;
    Opcode::Type   currentMnemo;
    Data           rom;
    LabelToAddress labelToAddress;
    LabelOffsets   labelOffsetTable;
    UserSection*   currentUserSection;
    Section        sections[1];
    UserSections   userSections;
};

using Directive = sys::MaybeError (*)(Context& ctx);
using Directives = std::map<std::string_view, Directive>;

Directives directives;

}  // namespace

static sys::MaybeError parseSectionDirective(Context& ctx);

static void initializeAssembler()
{
    const auto opcodes = gb.cpu.isa.getOpcodes();

    for (size_t i = 0; i < 512; ++i)
    {
        const auto& opcode = opcodes[i];
        mapping[opcode.mnemo].push_back(&opcode);
    }

#define OPCODE_MAPPING(STR, OPTYPE) \
    {#STR, Opcode::OPTYPE}

    opcodeType = {
        OPCODE_MAPPING(adc, ADC),
        OPCODE_MAPPING(add, ADD),
        OPCODE_MAPPING(and, AND),
        OPCODE_MAPPING(bit, BIT),
        OPCODE_MAPPING(call, CALL),
        OPCODE_MAPPING(ccf, CCF),
        OPCODE_MAPPING(cp, CP),
        OPCODE_MAPPING(cpl, CPL),
        OPCODE_MAPPING(daa, DAA),
        OPCODE_MAPPING(dec, DEC),
        OPCODE_MAPPING(di, DI),
        OPCODE_MAPPING(ei, EI),
        OPCODE_MAPPING(halt, HALT),
        OPCODE_MAPPING(inc, INC),
        OPCODE_MAPPING(jp, JP),
        OPCODE_MAPPING(jr, JR),
        OPCODE_MAPPING(ld, LD),
        OPCODE_MAPPING(ldh, LDH),
        OPCODE_MAPPING(nop, NOP),
        OPCODE_MAPPING(or, OR),
        OPCODE_MAPPING(pop, POP),
        OPCODE_MAPPING(push, PUSH),
        OPCODE_MAPPING(res, RES),
        OPCODE_MAPPING(ret, RET),
        OPCODE_MAPPING(reti, RETI),
        OPCODE_MAPPING(rl, RL),
        OPCODE_MAPPING(rla, RLA),
        OPCODE_MAPPING(rlc, RLC),
        OPCODE_MAPPING(rlca, RLCA),
        OPCODE_MAPPING(rr, RR),
        OPCODE_MAPPING(rra, RRA),
        OPCODE_MAPPING(rrc, RRC),
        OPCODE_MAPPING(rrca, RRCA),
        OPCODE_MAPPING(rst, RST),
        OPCODE_MAPPING(sbc, SBC),
        OPCODE_MAPPING(scf, SCF),
        OPCODE_MAPPING(set, SET),
        OPCODE_MAPPING(sla, SLA),
        OPCODE_MAPPING(sra, SRA),
        OPCODE_MAPPING(srl, SRL),
        OPCODE_MAPPING(stop, STOP),
        OPCODE_MAPPING(sub, SUB),
        OPCODE_MAPPING(swap, SWAP),
        OPCODE_MAPPING(xor, XOR),
    };

#undef OPCODE_MAPPING

    registers = {
        {"a",  Operand::A},
        {"b",  Operand::B},
        {"c",  Operand::C},
        {"d",  Operand::D},
        {"e",  Operand::E},
        {"h",  Operand::H},
        {"l",  Operand::L},
        {"af", Operand::AF},
        {"bc", Operand::BC},
        {"de", Operand::DE},
        {"hl", Operand::HL},
        {"sp", Operand::SP},
    };

    flags = {
        {"z", Operand::FlagZ},
        {"nz", Operand::FlagNZ},
        {"c", Operand::FlagC},
        {"nc", Operand::FlagNC},
    };

    directives = {
        {"SECTION", &parseSectionDirective},
    };
}

static const Token* peek(Context& ctx)
{
    return ctx.tokenIndex >= ctx.tokens.size() ? nullptr : &ctx.tokens[ctx.tokenIndex];
}

static const Token* peekNext(Context& ctx)
{
    return ctx.tokenIndex + 1 >= ctx.tokens.size() ? nullptr : &ctx.tokens[ctx.tokenIndex + 1];
}

static void advance(Context& ctx)
{
    ++ctx.tokenIndex;
}

namespace
{

struct OperandData final
{
    bool                 hasImmediate:1;
    bool                 immediateIndirect:1;
    Maybe<Operand::Type> maybeType;
    uint16_t             value;
    std::string          label;
};

using MaybeOperandData = std::expected<OperandData, std::string>;

}  // namespace

#define TOKEN_IS_TYPE(TOKEN, TYPE) \
    ({ TOKEN and TOKEN->type == Token::Type::TYPE; })

#define REQUIRE_TOKEN_TYPE(TOKEN, TYPE, ...) \
    do \
    { \
        if (not TOKEN_IS_TYPE(TOKEN, TYPE)) [[unlikely]] \
        { \
            return error(__VA_ARGS__); \
        } \
    } \
    while (0)

#define REQUIRE(CONDITION, ...) \
    do \
    { \
        if (not (CONDITION)) [[unlikely]] \
        { \
            return error(__VA_ARGS__); \
        } \
    } \
    while (0)

#define REQUIRE_FALSE(CONDITION, ...) \
    do \
    { \
        if (CONDITION) [[unlikely]] \
        { \
            return error(__VA_ARGS__); \
        } \
    } \
    while (0)

template <typename T, typename U>
static auto findValue(const T& map, U&& value) -> const typename T::mapped_type*
{
    auto it = map.find(std::forward<U>(value));
    if (it == map.end()) [[unlikely]]
    {
        return nullptr;
    }
    return &it->second;
}

using MaybeToken = std::expected<const Token*, std::string>;

#define PEEK_TOKEN() \
    ({ \
        auto token = peek(ctx); \
        REQUIRE(token, "Unexpected EOF"); \
        token; \
    })

#define POP_TOKEN_TYPE_ANY(TYPES, FMT, ...) \
    ({ \
        auto token = POP_TOKEN(); \
        bool found = false; \
        auto list = TYPES; \
        for (const auto type : list) \
        { \
            if (token->type == type) \
            { \
                found = true; \
                break; \
            } \
        } \
        REQUIRE(found, FMT __VA_OPT__(,) __VA_ARGS__); \
        token; \
    })

#define POP_TOKEN() \
    ({ auto token = PEEK_TOKEN(); advance(ctx); token; })

#define PEEK_TOKEN_WITH_TYPE(TYPE, FMT, ...) \
    ({ \
        auto token = PEEK_TOKEN(); \
        REQUIRE_TOKEN_TYPE(token, TYPE, FMT __VA_OPT__(,) __VA_ARGS__); \
        token; \
    })

#define POP_TOKEN_WITH_TYPE(TYPE, FMT, ...) \
    ({ \
        auto token = PEEK_TOKEN_WITH_TYPE(TYPE, FMT __VA_OPT__(,) __VA_ARGS__); \
        advance(ctx); \
        token; \
    })

#define SKIP_WHITESPACE() \
    ({ \
        bool gotWhitespace = false; \
        const Token* token = nullptr; \
        while ((token = peek(ctx)) and token->type == Token::Type::Whitespace) \
        { \
            gotWhitespace = true; \
            advance(ctx); \
        } \
        gotWhitespace; \
    })

#define EXPAND(...) \
    __VA_ARGS__

template <typename T>
static std::expected<T, std::string> parseInteger(Context& ctx)
{
    const auto number = POP_TOKEN_TYPE_ANY(
        EXPAND({interpreter::Token::Type::IntLiteral, interpreter::Token::Type::Identifier}),
        "Expected integer, got \"{}\"", token->value);

    T value = 0;

    auto res = std::from_chars(number->value.begin(), number->value.end(), value, 16);

    if (res.ec == std::errc::invalid_argument) [[unlikely]]
    {
        return error("Internal error: not a number: {}", number->value);
    }
    else if (res.ec == std::errc::result_out_of_range) [[unlikely]]
    {
        return error("Number out of valid range: {}", number->value);
    }

    return value;
}

static sys::MaybeError addUserSection(Context& ctx, const std::string_view& name, const std::string_view& sectionType, uint16_t address)
{
    if (sectionType != "ROM0")
    {
        return error("Unsupported section type: {}", sectionType);
    }

    if (ctx.currentUserSection)
    {
        ctx.currentUserSection->currentOffset = ctx.dataOffset;
    }

    auto result = ctx.userSections.emplace(name, UserSection{.section = &ctx.sections[0], .currentOffset = address});

    REQUIRE(result.second, "Section \"{}\" already defined", name);

    ctx.currentUserSection = &result.first->second;
    ctx.dataOffset = address;

    return {};
}

static sys::MaybeError parseSectionDirective(Context& ctx)
{
    auto name = POP_TOKEN_WITH_TYPE(StringLiteral, "Expected section name, got \"{}\"", token->value);
    SKIP_WHITESPACE();
    POP_TOKEN_WITH_TYPE(Comma, "Expected comma after section name, got \"{}\"", token->value);
    SKIP_WHITESPACE();
    auto type = POP_TOKEN_WITH_TYPE(Identifier, "Expected section type, got \"{}\"", token->value);
    POP_TOKEN_WITH_TYPE(LeftSquareBracket, "Expected \"[\", got \"{}\"", token->value);
    SKIP_WHITESPACE();
    POP_TOKEN_WITH_TYPE(Dollar, "Expected dollar, got \"{}\"", token->value);

    auto maybeValue = parseInteger<uint16_t>(ctx);

    if (not maybeValue)
    {
        return std::unexpected(maybeValue.error());
    }

    SKIP_WHITESPACE();
    POP_TOKEN_WITH_TYPE(RightSquareBracket, "Expected \"]\", got \"{}\"", token->value);

    return addUserSection(ctx, name->value, type->value, *maybeValue);
}

static MaybeOperandData parseOperand(Context& ctx)
{
    auto token = peek(ctx);

    bool indirect = false;
    switch (token->type)
    {
        case Token::Type::LeftSquareBracket:
        {
            advance(ctx);
            token = PEEK_TOKEN();
            indirect = true;
            switch (token->type)
            {
                case Token::Type::Dollar:
                    goto intLiteral;
                case Token::Type::Identifier:
                    goto identifier;
                default:
                    return error("Unexpected token after \"[\": \"{}\"", token->value);
            }
        }

        identifier:
        case Token::Type::Identifier:
        {
            utils::Maybe<Operand::Type> type;
            if (ctx.currentMnemo == Opcode::JP or
                ctx.currentMnemo == Opcode::JR or
                ctx.currentMnemo == Opcode::CALL)
            {
                if (auto flag = findValue(flags, token->value))
                {
                    type = *flag;
                }
            }
            else
            {
                if (auto reg = findValue(registers, token->value))
                {
                    type = *reg;
                }
            }
            advance(ctx);
            if (indirect)
            {
                POP_TOKEN_WITH_TYPE(RightSquareBracket, "Expected \"]\" after identifier, got \"{}\"", token->value);
            }
            if (not type)
            {
                return OperandData{
                    .hasImmediate = true,
                    .immediateIndirect = indirect,
                    .maybeType = {},
                    .value = 0,
                    .label = std::string(token->value),
                };
            }
            return OperandData{
                .hasImmediate = false,
                .immediateIndirect = indirect,
                .maybeType = type,
                .value = 0,
            };
        }

        intLiteral:
        case Token::Type::Dollar:
        {
            advance(ctx);
            auto token = PEEK_TOKEN();
            int mult = 1;
            switch (token->type)
            {
                case Token::Type::Sub:
                {
                    mult = -1;
                    REQUIRE_FALSE(indirect, "Expected integer after \"$\", got \"{}\"", token->value);
                    advance(ctx);
                    token = PEEK_TOKEN();
                    REQUIRE(TOKEN_IS_TYPE(token, IntLiteral) or TOKEN_IS_TYPE(token, Identifier),
                        "Expected integer after \"-\", got \"{}\"", token->value);
                    [[fallthrough]];
                }

                case Token::Type::IntLiteral:
                case Token::Type::Identifier:
                {
                    uint16_t value;
                    if (mult == -1)
                    {
                        auto tmp = parseInteger<uint8_t>(ctx);
                        if (not tmp)
                        {
                            return std::unexpected(tmp.error());
                        }
                        value = *tmp * -1;
                    }
                    else
                    {
                        auto tmp = parseInteger<uint16_t>(ctx);
                        if (not tmp)
                        {
                            return std::unexpected(tmp.error());
                        }
                        value = *tmp;
                    }
                    if (indirect)
                    {
                        POP_TOKEN_WITH_TYPE(RightSquareBracket, "Expected \"]\" after integer, got \"{}\"", token->value);
                    }
                    return OperandData{
                        .hasImmediate = true,
                        .immediateIndirect = indirect,
                        .maybeType = {},
                        .value = value,
                    };
                }

                default:
                    return error("Expected integer after $, got {}", token->value);
            }
            break;
        }

        default:
            return error("Incorrect token: {}", *token);
    }
    return {};
}

using MaybeOpcode = std::expected<const Opcode*, std::string>;

static bool opcodeMatchesWithOperands(const Opcode& isaOpcode, const std::vector<OperandData>& operands)
{
    if (isaOpcode.opCount != operands.size())
    {
        return false;
    }

    if (isaOpcode.opCount == 0)
    {
        return true;
    }

    bool matched = false;
    for (uint8_t i = 0; i < isaOpcode.opCount; ++i)
    {
        const auto isaOperand = isaOpcode.op[i];
        const auto& operand = operands[i];

        if (isaOperand.indirect == operand.immediateIndirect and
            isaOperand.action == Operand::Action::None)
        {
            if (operand.maybeType and isaOperand.type == *operand.maybeType)
            {
                matched = true;
            }
            else if (operand.hasImmediate)
            {
                if (isaOperand.type == Operand::Builtin and isaOperand.value == operand.value)
                {
                    matched = true;
                }
                else if (isaOperand.type == Operand::ImmS8)
                {
                    matched = true;
                }
                else if ((isaOperand.type == Operand::ImmU8 or
                    isaOperand.type == Operand::Addr8) and
                    operand.value < UINT8_MAX)
                {
                    matched = true;
                }
                else if (isaOperand.type == Operand::Imm16 or
                    isaOperand.type == Operand::Addr16)
                {
                    matched = true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return matched;
}

static MaybeOpcode findOpcode(Context&, Opcode::Type mnemo, const std::vector<OperandData>& operands)
{
    const auto maybeOpcodes = findValue(mapping, mnemo);

    REQUIRE(maybeOpcodes, "Internal error; cannot find opcodes for {}", mnemo);

    for (const auto opcode : *maybeOpcodes)
    {
        if (opcodeMatchesWithOperands(*opcode, operands))
        {
            return opcode;
        }
    }

    return error("Cannot find matching opcode");
}

static sys::MaybeError parseLabel(Context& ctx)
{
    const auto token = POP_TOKEN_WITH_TYPE(Identifier, "Expected identifier, got \"{}\"", token->value);
    POP_TOKEN_WITH_TYPE(Colon, "Expected \":\" after label name");

    REQUIRE_FALSE(registers.contains(token->value) or flags.contains(token->value), "Name {} is reserved", token->value);

    auto res = ctx.labelToAddress.emplace(std::string(token->value), ctx.dataOffset);

    REQUIRE(res.second, "Label {} was already defined", token->value);

    return {};
}

static sys::MaybeError parseInstruction(Context& ctx)
{
    auto token = POP_TOKEN_WITH_TYPE(Identifier, "Expected identifier, got \"{}\"", token->value);

    if (auto directive = findValue(directives, token->value))
    {
        SKIP_WHITESPACE();
        return (*directive)(ctx);
    }

    auto maybeMnemo = findValue(opcodeType, token->value);

    REQUIRE(maybeMnemo, "Expected instruction name, got: {}", token->value);

    const auto mnemo = ctx.currentMnemo = *maybeMnemo;

    auto gotWhitespace = SKIP_WHITESPACE();

    std::vector<OperandData> operands;

    while ((token = peek(ctx)))
    {
        switch (token->type)
        {
            case Token::Type::Whitespace:
                advance(ctx);
                break;

            case Token::Type::Comma:
                if (operands.empty()) [[unlikely]]
                {
                    goto unexpectedToken;
                }
                advance(ctx);
                break;

            case Token::Type::Dollar:
            case Token::Type::Identifier:
            case Token::Type::LeftSquareBracket:
            {
                if (not gotWhitespace)
                {
                    return error("Expected whitespace before {}", token->value);
                }
                auto operand = parseOperand(ctx);
                REQUIRE(operand, std::move(operand.error()));
                operands.push_back(*operand);
                break;
            }

            case Token::Type::Comment:
            case Token::Type::Newline:
            case Token::Type::End:
                goto exitLoop;

            unexpectedToken:
            default:
                return error("Unexpected token: \"{}\"", token->value);
        }
    }

exitLoop:
    const auto maybeOpcode = findOpcode(ctx, mnemo, operands);

    REQUIRE(maybeOpcode, maybeOpcode.error());

    const auto opcode = *maybeOpcode;

    if (opcode - gb.cpu.isa.getOpcodes() >= 256)
    {
        ctx.rom[ctx.dataOffset++] = 0xcb;
    }
    ctx.rom[ctx.dataOffset++] = opcode->value;

    for (uint8_t i = 0; i < opcode->opCount; ++i)
    {
        auto op = opcode->op[i];
        switch (op.type)
        {
            case Operand::ImmS8:
            case Operand::ImmU8:
            case Operand::Addr8:
                if (not operands[i].label.empty())
                {
                    ctx.labelOffsetTable.push_back(LabelOffset{
                        .relative = true,
                        .offset   = ctx.dataOffset,
                        .label    = operands[i].label,
                    });
                }
                ctx.rom[ctx.dataOffset++] = operands[i].value;
                break;
            case Operand::Imm16:
            case Operand::Addr16:
                if (not operands[i].label.empty())
                {
                    ctx.labelOffsetTable.push_back(LabelOffset{
                        .relative = false,
                        .offset   = ctx.dataOffset,
                        .label    = operands[i].label,
                    });
                }
                ctx.rom[ctx.dataOffset++] = utils::lsb(operands[i].value);
                ctx.rom[ctx.dataOffset++] = utils::msb(operands[i].value);
                break;
            default:
                break;
        }
    }

    return {};
}

static void writeHeader(Context& ctx)
{
    auto& header = *reinterpret_cast<memory::CartridgeHeader*>(ctx.rom.data());

    header.type = memory::CartridgeType::ROM_ONLY;
    header.romSize = 0;
    header.ramSize = 0;
}

MaybeRom assemble(const std::string_view& text)
{
    auto result = interpreter::parse(text, ';', true);

    REQUIRE(result, "Failed to parse: {}", result.error());

    if (mapping.empty()) [[unlikely]]
    {
        initializeAssembler();
    }

    Context ctx{
        .tokenIndex = 0,
        .tokens = std::move(*result),
        .opcodes = gb.cpu.isa.getOpcodes(),
        .dataOffset = 0,
        .currentMnemo = Opcode::Type::ILL,
        .rom{},
        .labelToAddress{},
        .labelOffsetTable{},
        .currentUserSection = nullptr,
        .sections = {
            Section{.type = ROM0, .currentOffset = 0},
        }
    };

    ctx.rom.reserve(32_KiB);

    while (const auto token = peek(ctx))
    {
        switch (token->type)
        {
            case Token::Type::Identifier:
            {
                auto nextToken = peekNext(ctx);
                if (nextToken and nextToken->type == Token::Type::Colon)
                {
                    if (auto res = parseLabel(ctx); not res)
                    {
                        return error(std::move(res.error()));
                    }
                }
                else if (auto res = parseInstruction(ctx); not res)
                {
                    return error(std::move(res.error()));
                }
                break;
            }

            case Token::Type::Comment:
            case Token::Type::End:
            case Token::Type::Whitespace:
            case Token::Type::Newline:
                advance(ctx);
                break;

            default:
                return error("Unexpected token: \"{}\"", token->value);
        }
    }

    for (const auto& e : ctx.labelOffsetTable)
    {
        const auto& label = e.label;
        const auto offset = e.offset;

        const auto labelAddress = findValue(ctx.labelToAddress, label);

        REQUIRE(labelAddress, "Undefined reference to {}", label);

        if (e.relative)
        {
            long relative = static_cast<long>(*labelAddress) - static_cast<long>(offset) - 1;
            REQUIRE_FALSE(relative > INT8_MAX or relative < INT8_MIN,
                "Cannot perform relative jump, offset is too long ({})", relative);
            ctx.rom[offset] = static_cast<int8_t>(relative);
        }
        else
        {
            size_t addr = offset;
            ctx.rom[addr++] = utils::lsb(*labelAddress);
            ctx.rom[addr++] = utils::msb(*labelAddress);
        }
    }

    writeHeader(ctx);

    return std::move(ctx.rom);
}

}  // namespace cpu::isa
