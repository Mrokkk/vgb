#include "printers.hpp"

#include <fmt/base.h>

#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"

fmt::format_context::iterator fmt::formatter<cpu::isa::InstructionData>::format(const cpu::isa::InstructionData& data, format_context& ctx) const
{
    auto it = ctx.out();
    uint16_t i;
    for (i = 0; i < data.bytes; ++i)
    {
        it = fmt::format_to(it, "{:02x} ", data.data[i]);
    }
    for (; i < sizeof(data.data); ++i)
    {
        it = fmt::format_to(it, "   ", data.data[i]);
    }
    return it;
}

fmt::format_context::iterator fmt::formatter<cpu::isa::Opcode::Type>::format(cpu::isa::Opcode::Type type, format_context& ctx) const
{
#define MNEMO(UPPER, LOWER) case cpu::isa::Opcode::Type::UPPER: return format_to(ctx.out(), #LOWER);
#define MNEMO_ILL MNEMO
    switch (type)
    {
#include "cpu/isa/mnemos.hpp"
    }
    return format_to(ctx.out(), "Unknown({})", int(type));
#undef MNEMO
#undef MNEMO_ILL
}

fmt::format_context::iterator fmt::formatter<cpu::isa::Operand::Type>::format(cpu::isa::Operand::Type type, format_context& ctx) const
{
#define CONVERT(TYPE) \
    case cpu::isa::Operand::Type::TYPE: \
        return format_to(ctx.out(), #TYPE)
    switch (type)
    {
        CONVERT(None);
        CONVERT(Builtin);
        CONVERT(ImmS8);
        CONVERT(ImmU8);
        CONVERT(Imm16);
        CONVERT(Addr8);
        CONVERT(Addr16);
        CONVERT(A);
        CONVERT(B);
        CONVERT(C);
        CONVERT(D);
        CONVERT(E);
        CONVERT(H);
        CONVERT(L);
        CONVERT(AF);
        CONVERT(BC);
        CONVERT(DE);
        CONVERT(HL);
        CONVERT(SP);
        CONVERT(SP_Plus_ImmS8);
        CONVERT(FlagZ);
        CONVERT(FlagNZ);
        CONVERT(FlagC);
        CONVERT(FlagNC);
    }
    return format_to(ctx.out(), "Unknown({})", int(type));
#undef CONVERT
}
