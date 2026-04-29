#pragma once

#include <fmt/base.h>

#include "cpu/isa/opcode.hpp"

namespace cpu::isa
{

struct Decode
{
    const Opcode&          opcode;
    const InstructionData& data;
};

Decode decode(const Opcode& opcode, const InstructionData& data);

}  // namespace cpu::isa

template <>
struct fmt::formatter<cpu::isa::Decode> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::isa::Decode& d, format_context& ctx) const;
};
