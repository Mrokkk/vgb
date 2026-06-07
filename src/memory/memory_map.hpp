#pragma once

#include <cstdint>

namespace memory
{

struct MemoryRange
{
    uint32_t start;
    uint32_t end;
};

#define DEFINE_MEMORY_RANGE(NAME, START, END) \
    static constexpr inline MemoryRange NAME{START, END}

#define DEFINE_MEMORY_SINGLE(NAME, ADDR) \
    static constexpr inline uint32_t NAME = ADDR

struct Map
{
    DEFINE_MEMORY_RANGE(BOOT_ROM,    0x0000, 0x0100);
    DEFINE_MEMORY_RANGE(ROM,         0x0100, 0x8000);
    DEFINE_MEMORY_RANGE(VRAM,        0x8000, 0xa000);
    DEFINE_MEMORY_RANGE(EXT_RAM,     0xa000, 0xc000);
    DEFINE_MEMORY_RANGE(BASE_WRAM,   0xc000, 0xd000);
    DEFINE_MEMORY_RANGE(BANKED_WRAM, 0xd000, 0xe000);
    DEFINE_MEMORY_RANGE(OAM,         0xfe00, 0xfea0);
    DEFINE_MEMORY_RANGE(INVALID,     0xfea0, 0xff00);
    DEFINE_MEMORY_RANGE(IO,          0xff00, 0xff80);
    DEFINE_MEMORY_RANGE(HRAM,        0xff80, 0xffff);
    DEFINE_MEMORY_SINGLE(IE,         0xffff);
};

}  // namespace memory
