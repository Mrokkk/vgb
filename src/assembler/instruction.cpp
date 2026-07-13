#include <cstdint>
#include <map>
#include <vector>

#include "assembler/argument.hpp"
#include "assembler/context.hpp"
#include "assembler/glue.hpp"
#include "assembler/helpers.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"
#include "cpu/isa/printers.hpp"
#include "game_boy.hpp"

using namespace cpu::isa;

namespace assembler
{

using OpcodeTypeToOpcodes = std::map<Opcode::Type, std::vector<const cpu::isa::Opcode*>>;

static OpcodeTypeToOpcodes mapping;

static void initialize()
{
    if (mapping.empty())
    {
        const auto opcodes = gb.cpu.isa.getOpcodes();

        for (size_t i = 0; i < 512; ++i)
        {
            const auto& opcode = opcodes[i];
            mapping[opcode.mnemo].push_back(&opcode);
        }
    }
}

static void serializeInstruction(Context& ctx, const Opcode& opcode, const Arguments& args)
{
    if (&opcode - gb.cpu.isa.getOpcodes() >= 256)
    {
        ctx.rom[ctx.dataOffset++] = 0xcb;
    }
    ctx.rom[ctx.dataOffset++] = opcode.value;
    for (uint8_t i = 0; i < opcode.opCount; ++i)
    {
        auto op = opcode.op[i];
        const auto& arg = args[i];
        switch (op.type)
        {
            case Operand::ImmS8:
            case Operand::ImmU8:
            case Operand::Addr8:
            case Operand::SP_Plus_ImmS8:
                if (arg.getType() == Argument::Label)
                {
                    ctx.labelOffsetTable.push_back(LabelOffset{
                        .relative = true,
                        .offset   = ctx.dataOffset,
                        .label    = std::string(arg.label()),
                    });
                    ctx.rom[ctx.dataOffset++] = 0;
                }
                else if (arg.getType() == Argument::Expression or arg.getType() == Argument::SpPlusExpression)
                {
                    ctx.rom[ctx.dataOffset++] = arg.expression().value();
                }
                else [[unlikely]]
                {
                    reportError(ctx, "Internal error: incorrect argument type: {}", (int)arg.getType());
                    return;
                }
                break;
            case Operand::Imm16:
            case Operand::Addr16:
                if (arg.getType() == Argument::Label)
                {
                    ctx.labelOffsetTable.push_back(LabelOffset{
                        .relative = false,
                        .offset   = ctx.dataOffset,
                        .label    = std::string(arg.label()),
                    });
                    ctx.rom[ctx.dataOffset++] = 0;
                    ctx.rom[ctx.dataOffset++] = 0;
                }
                else if (arg.getType() == Argument::Expression)
                {
                    ctx.rom[ctx.dataOffset++] = utils::lsb(arg.expression().value());
                    ctx.rom[ctx.dataOffset++] = utils::msb(arg.expression().value());
                }
                else [[unlikely]]
                {
                    reportError(ctx, "internal error: incorrect argument type: {}", (int)arg.getType());
                    return;
                }
                break;
            default:
                break;
        }
    }
}

static bool doesOperandMatch(const Opcode::Type mnemo, Operand isaOperand, const Argument& arg)
{
    if (isaOperand.indirect != arg.isIndirect() or
        isaOperand.action != arg.getAction())
    {
        return false;
    }

    if (arg.getType() == Argument::Operand)
    {
        if (isaOperand.type == arg.operand())
        {
            return true;
        }

        if ((mnemo == Opcode::JP or
            mnemo == Opcode::JR or
            mnemo == Opcode::CALL) and
            arg.operand() == Operand::C and
            isaOperand.type == Operand::FlagC)
        {
            return true;
        }

        return false;
    }

    if (arg.getType() == Argument::Label)
    {
        if (isaOperand.type == Operand::ImmS8 or
            isaOperand.type == Operand::Imm16 or
            isaOperand.type == Operand::Addr16)
        {
            return true;
        }
    }

    if (arg.getType() == Argument::Expression)
    {
        if (isaOperand.type == Operand::Builtin and isaOperand.value == arg.expression().value())
        {
            return true;
        }

        if (isaOperand.type == Operand::ImmS8 and
            arg.expression().value() >= INT8_MIN and
            arg.expression().value() <= INT8_MAX)
        {
            return true;
        }

        if ((isaOperand.type == Operand::ImmU8 or
            isaOperand.type == Operand::Addr8))
        {
            if (mnemo == Opcode::Type::LDH and (arg.expression().value() & 0xff00) == 0xff00)
            {
                return true;
            }

            if (arg.expression().value() >= 0 and
                arg.expression().value() <= UINT8_MAX)
            {
                return true;
            }
        }
        else if ((isaOperand.type == Operand::Imm16 or
            isaOperand.type == Operand::Addr16) and
            arg.expression().value() >= 0 and
            arg.expression().value() <= UINT16_MAX)
        {
            return true;
        }
    }

    if (arg.getType() == Argument::SpPlusExpression)
    {
        if (isaOperand.type == Operand::SP_Plus_ImmS8 and
            arg.expression().value() >= INT8_MIN and
            arg.expression().value() <= INT8_MAX)
        {
            return true;
        }
    }

    return false;
}

static bool doesOpcodeMatchWithArguments(const Opcode& opcode, const Arguments& args)
{
    if (opcode.opCount != args.size())
    {
        return false;
    }

    if (opcode.opCount == 0)
    {
        return true;
    }

    for (uint8_t i = 0; i < opcode.opCount; ++i)
    {
        const auto isaOperand = opcode.op[i];
        const auto& arg = args[i];

        if (not doesOperandMatch(opcode.mnemo, isaOperand, arg))
        {
            return false;
        }
    }

    return true;
}

void handleInstruction(Context& ctx, Opcode::Type mnemo, const Arguments& args)
{
    initialize();

    const auto maybeOpcodes = findValue(mapping, mnemo);

    if (not maybeOpcodes) [[unlikely]]
    {
        reportError(ctx, "internal error; cannot find opcodes for {}", mnemo);
        return;
    }

    for (const auto opcode : *maybeOpcodes)
    {
        if (doesOpcodeMatchWithArguments(*opcode, args))
        {
            serializeInstruction(ctx, *opcode, args);
            return;
        }
    }

    reportError(ctx, "instruction {} with incorrect operand(s)", mnemo);
}

}  // namespace assembler
