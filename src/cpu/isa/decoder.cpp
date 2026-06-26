#include "decoder.hpp"

#include <fmt/base.h>
#include <fmt/fmt_ext.h>
#include <string_view>

#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"
#include "cpu/sm83.hpp"

namespace cpu::isa
{

constexpr static std::string_view registerNames[] = {
    "a",
    "b",
    "c",
    "d",
    "e",
    "h",
    "l",
    "af",
    "bc",
    "de",
    "hl",
    "sp",
};

constexpr static std::string_view flagNames[] = {
    "z",
    "nz",
    "c",
    "nc",
};

static auto decode(const Decode& d, fmt::format_context& ctx)
{
    const auto& opcode = d.opcode;
    const auto& data = d.data;
    auto it = ctx.out();

    it = fmt::format_to(it, "{}", opcodeName(opcode.mnemo));

    if (opcode.opCount)
    {
        it = fmt::format_to(it, " ");
    }

    for (uint8_t i = 0; i < opcode.opCount; ++i)
    {
        auto& op = d.opcode.op[i];
        switch (op.type)
        {
            case Operand::Builtin:
                it = fmt::format_to(it, "${:x}", op.value);
                break;

            case Operand::ImmS8:
                it = fmt::format_to(it, "${:x}", data.immS8());
                break;

            case Operand::ImmU8:
                it = fmt::format_to(it, "${:x}", data.immU8());
                break;

            case Operand::Imm16:
                it = fmt::format_to(it, "${:x}", data.immU16());
                break;

            case Operand::Addr8:
                if (op.indirect)
                {
                    it = fmt::format_to(it, "[");
                }
                it = fmt::format_to(it, "$ff00+${:02x}", data.immU8());
                if (op.indirect)
                {
                    it = fmt::format_to(it, "]");
                }
                break;

            case Operand::Addr16:
                if (op.indirect)
                {
                    it = fmt::format_to(it, "[");
                }
                it = fmt::format_to(it, "${:04x}", data.immU16());
                if (op.indirect)
                {
                    it = fmt::format_to(it, "]");
                }
                break;

            case Operand::A ... Operand::SP:
                if (op.indirect)
                {
                    it = fmt::format_to(it, "[");
                }

                it = fmt::format_to(it, "{}{}",
                    registerNames[(int)op.type - (int)Operand::A],
                    op.action == Operand::Action::Decrement ? "-" : op.action == Operand::Action::Increment ? "+" : "");

                if (op.indirect)
                {
                    it = fmt::format_to(it, "]");
                }
                break;

            case Operand::SP_Plus_ImmS8:
                // TODO
                break;

            case Operand::FlagZ ... Operand::FlagNC:
                it = fmt::format_to(it, "{}", flagNames[(int)op.type - (int)Operand::FlagZ]);
                break;

            default:
            case Operand::None:
                break;
        }

        if (i != opcode.opCount - 1)
        {
            it = fmt::format_to(it, ", ");
        }
    }

    return it;
}

Decode decode(const Opcode& opcode, const InstructionData& data)
{
    return Decode{opcode, data};
}

void disassemble(DisassembleContext& ctx)
{
    auto& cpu = ctx.cpu;
    bool prefixed = false;

    ctx.data.clear();
    uint8_t pcValue = ctx.data.appendOpcodeByte(cpu.mem.load8(ctx.pc++));

    if (pcValue == 0xcb)
    {
        prefixed = true;
        pcValue = ctx.data.appendOpcodeByte(cpu.mem.load8(ctx.pc++));
    }

    const auto& opcode = cpu.isa.getOpcode(prefixed, pcValue);

    ctx.opcode = &opcode;

    for (uint8_t i = ctx.data.bytes; i < opcode.bytes; ++i)
    {
        ctx.data.appendImmByte(cpu.mem.load8(ctx.pc++));
    }

    ctx.disassembled = fmt::format_to_string("{}", cpu::isa::decode(opcode, ctx.data));
}

}  // namespace cpu::isa

fmt::format_context::iterator fmt::formatter<cpu::isa::Decode>::format(const cpu::isa::Decode& d, format_context& ctx) const
{
    return decode(d, ctx);
}
