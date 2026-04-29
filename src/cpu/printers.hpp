#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "cpu/fwd.hpp"

template <>
struct fmt::formatter<cpu::Exception> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::Exception& exc, format_context& ctx) const;
};

template <>
struct fmt::formatter<cpu::SM83> : fmt::formatter<string_view>
{
    format_context::iterator format(const cpu::SM83& cpu, format_context& ctx) const;
};
