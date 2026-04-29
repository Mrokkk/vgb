#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "cpu/isa/fwd.hpp"

template <>
struct fmt::formatter<cpu::isa::InstructionData> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::isa::InstructionData& exc, format_context& ctx) const;
};
