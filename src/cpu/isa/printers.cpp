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
#define CONVERT(TYPE) \
    case cpu::isa::Opcode::Type::TYPE: \
        return format_to(ctx.out(), #TYPE)
    switch (type)
    {
        CONVERT(ILL);
        CONVERT(ADC);
        CONVERT(ADD);
        CONVERT(AND);
        CONVERT(BIT);
        CONVERT(CALL);
        CONVERT(CCF);
        CONVERT(CP);
        CONVERT(CPL);
        CONVERT(DAA);
        CONVERT(DEC);
        CONVERT(DI);
        CONVERT(EI);
        CONVERT(HALT);
        CONVERT(INC);
        CONVERT(JP);
        CONVERT(JR);
        CONVERT(LD);
        CONVERT(LDH);
        CONVERT(NOP);
        CONVERT(OR);
        CONVERT(POP);
        CONVERT(PREFIX);
        CONVERT(PUSH);
        CONVERT(RES);
        CONVERT(RET);
        CONVERT(RETI);
        CONVERT(RL);
        CONVERT(RLA);
        CONVERT(RLC);
        CONVERT(RLCA);
        CONVERT(RR);
        CONVERT(RRA);
        CONVERT(RRC);
        CONVERT(RRCA);
        CONVERT(RST);
        CONVERT(SBC);
        CONVERT(SCF);
        CONVERT(SET);
        CONVERT(SLA);
        CONVERT(SRA);
        CONVERT(SRL);
        CONVERT(STOP);
        CONVERT(SUB);
        CONVERT(SWAP);
        CONVERT(XOR);
    }
    return format_to(ctx.out(), "Unknown({})", int(type));
#undef CONVERT
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
