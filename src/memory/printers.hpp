#pragma once

#pragma once

#include <fmt/base.h> // IWYU pragma: export

#include "memory/cartridge.hpp"

template <>
struct fmt::formatter<memory::CartridgeType> : fmt::formatter<string_view>
{
    format_context::iterator format(const memory::CartridgeType type, format_context& ctx) const;
};
