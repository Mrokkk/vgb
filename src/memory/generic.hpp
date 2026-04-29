#pragma once

#include <cstddef>
#include <cstdint>

#include "utils/inline.hpp"

namespace memory
{

template <size_t SIZE>
struct GenericIO
{
    ALWAYS_INLINE void store(uint8_t addr, uint8_t value)
    {
        const auto mask = roMasks[addr];
        const auto tmp = data[addr] & mask;
        data[addr] = tmp | (value & ~mask);
    }

    ALWAYS_INLINE uint8_t load(uint8_t addr) const
    {
        return data[addr];
    }

    uint8_t data[SIZE];
    uint8_t roMasks[SIZE];
};

template <size_t SIZE>
struct GenericRAM
{
    ALWAYS_INLINE void store(uint16_t addr, uint8_t value)
    {
        data[addr] = value;
    }

    ALWAYS_INLINE uint8_t load(uint16_t addr) const
    {
        return data[addr];
    }

    uint8_t data[SIZE];
};

}  // namespace memory
