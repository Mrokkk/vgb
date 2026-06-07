#pragma once

#include <cstdint>

#include "memory/generic.hpp"
#include "memory/memory_map.hpp"

namespace memory
{

struct Memory final
{
    Memory();
    ~Memory();

    void reset();

    uint8_t  load8(uint16_t addr) const;
    void     store8(uint16_t addr, uint8_t val);

    uint16_t load16(uint16_t addr) const;
    void     store16(uint16_t addr, uint16_t val);

    BankedMemory<Map::VRAM, 2>        vram;
    GenericRAM<Map::BASE_WRAM>        baseWorkRam;
    BankedMemory<Map::BANKED_WRAM, 7> bankedWorkRam;
    GenericRAM<Map::OAM>              oam;
    GenericRAM<Map::HRAM>             highRam;

private:
    bool mBootRomEnabled;
};

}  // namespace memory
