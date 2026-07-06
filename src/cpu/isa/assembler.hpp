#pragma once

#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

namespace cpu::isa
{

using MaybeRom = std::expected<std::vector<uint8_t>, std::string>;

MaybeRom assemble(const std::string_view& text);

}  // namespace cpu::isa
