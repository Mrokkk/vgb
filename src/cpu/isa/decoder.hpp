#pragma once

#include <string>

#include <fmt/base.h>

#include "cpu/fwd.hpp"
#include "cpu/isa/opcode.hpp"

namespace cpu::isa
{

struct Decode
{
    const Opcode&          opcode;
    const InstructionData& data;
};

Decode decode(const Opcode& opcode, const InstructionData& data);

struct DisassembleContext
{
    static DisassembleContext create(const SM83& cpu, uint16_t pc)
    {
        return DisassembleContext{
            .pc = pc,
            .cpu = cpu,
            .data{},
            .opcode = nullptr,
            .disassembled{}
        };
    }

    uint16_t        pc;
    const SM83&     cpu;
    InstructionData data;
    const Opcode*   opcode;
    std::string     disassembled;
};

void disassemble(DisassembleContext& ctx);

}  // namespace cpu::isa

template <>
struct fmt::formatter<cpu::isa::Decode> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::isa::Decode& d, format_context& ctx) const;
};
