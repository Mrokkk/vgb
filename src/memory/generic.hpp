#pragma once

#include <cstddef>
#include <cstdint>

#include "memory/memory_map.hpp"
#include "utils/inline.hpp"

namespace memory
{

template <size_t SIZE>
struct GenericIO
{
    GenericIO()
        : data{}
        , roMasks{}
    {
    }

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

template <MemoryRange RANGE>
struct GenericRAM
{
    ALWAYS_INLINE void store(uint16_t addr, uint8_t value)
    {
        data[addr - RANGE.start] = value;
    }

    ALWAYS_INLINE uint8_t load(uint16_t addr) const
    {
        return data[addr - RANGE.start];
    }

    ALWAYS_INLINE constexpr static uint16_t size()
    {
        return RANGE.size();
    }

    uint8_t data[size()];
};

template <MemoryRange RANGE, size_t BANKS>
struct BankedMemory
{
    ALWAYS_INLINE constexpr void store(uint16_t addr, uint8_t value)
    {
        data[addr - RANGE.start + bank * RANGE.size()] = value;
    }

    ALWAYS_INLINE constexpr uint8_t load(uint16_t addr) const
    {
        return data[addr - RANGE.start + bank * RANGE.size()];
    }

    ALWAYS_INLINE constexpr static uint16_t size()
    {
        return RANGE.size() * BANKS;
    }

    size_t bank = 0;
    uint8_t data[size()];
};

}  // namespace memory
