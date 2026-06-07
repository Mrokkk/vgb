#pragma once

#include <cstdint>

namespace memory
{

struct MemoryRange final
{
    const int      id;
    const uint32_t start;
    const uint32_t end;

    constexpr uint16_t relative(uint16_t addr) const
    {
        return addr - start;
    }

    constexpr uint16_t size() const
    {
        return end - start;
    }
};

#define DEFINE_MEMORY_RANGE(NAME, ID, START, END) \
    static constexpr inline MemoryRange NAME{ID, START, END}

struct Map final
{
    DEFINE_MEMORY_RANGE(BOOT_ROM,      0, 0x00000, 0x00100);
    DEFINE_MEMORY_RANGE(ROM,           1, 0x00100, 0x08000);
    DEFINE_MEMORY_RANGE(VRAM,          2, 0x08000, 0x0a000);
    DEFINE_MEMORY_RANGE(EXT_RAM,       3, 0x0a000, 0x0c000);
    DEFINE_MEMORY_RANGE(BASE_WRAM,     4, 0x0c000, 0x0d000);
    DEFINE_MEMORY_RANGE(BANKED_WRAM,   5, 0x0d000, 0x0e000);
    DEFINE_MEMORY_RANGE(MIRROR,        6, 0x0e000, 0x0fe00);
    DEFINE_MEMORY_RANGE(OAM,           7, 0x0fe00, 0x0fea0);
    DEFINE_MEMORY_RANGE(INVALID,       8, 0x0fea0, 0x0ff00);
    DEFINE_MEMORY_RANGE(IO,            9, 0x0ff00, 0x0ff80);
    DEFINE_MEMORY_RANGE(HRAM,         10, 0x0ff80, 0x0ffff);
    DEFINE_MEMORY_RANGE(IE,           11, 0x0ffff, 0x10000);
    static constexpr inline uint32_t ADDRESS_SPACE_SIZE = 0x10000;
};

}  // namespace memory
