#pragma once

#include <cstdint>

#include "utils/inline.hpp"

namespace utils
{

ALWAYS_INLINE uint16_t le16(const uint8_t* ptr)
{
    return (uint16_t)ptr[0] | ((uint16_t)ptr[1]);
}

ALWAYS_INLINE uint16_t le16(uint8_t low, uint8_t high)
{
    return (uint16_t)low | ((uint16_t)high << 8);
}

ALWAYS_INLINE uint8_t msb(uint16_t val)
{
    return val >> 8;
}

ALWAYS_INLINE uint8_t lsb(uint16_t val)
{
    return val & 0xff;
}

}  // namespace utils
