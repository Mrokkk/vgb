#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "cpu/isa/fwd.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/operand.hpp"

template <>
struct fmt::formatter<cpu::isa::InstructionData> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::isa::InstructionData& data, format_context& ctx) const;
};

template <>
struct fmt::formatter<cpu::isa::Opcode::Type> : fmt::formatter<string_view>
{
    format_context::iterator format(cpu::isa::Opcode::Type type, format_context& ctx) const;
};

template <>
struct fmt::formatter<cpu::isa::Operand::Type> : fmt::formatter<string_view>
{
    format_context::iterator format(cpu::isa::Operand::Type type, format_context& ctx) const;
};
