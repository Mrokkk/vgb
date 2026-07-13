#pragma once

#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

#include "assembler/error.hpp"

namespace assembler
{

using MaybeRom = std::expected<std::vector<uint8_t>, Errors>;

MaybeRom assemble(const std::string_view& fileName);

}  // namespace assembler
