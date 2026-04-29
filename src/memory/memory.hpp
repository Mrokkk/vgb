#pragma once

#include <cstdint>

#include "memory/cartridge.hpp"

namespace memory
{

struct Memory
{
    Memory();
    ~Memory();

    void loadCartridge(void* data);

    uint8_t  load8(uint16_t addr) const;
    uint16_t load16(uint16_t addr) const;
    void     store8(uint16_t addr, uint8_t val);
    void     store16(uint16_t addr, uint16_t val);

private:
    bool      mBootRomEnabled;
    Cartridge mCartridge;
    uint8_t   mBaseWRam[0xd000 - 0xc000];
    uint8_t   mSwitchableWRam[(0xe000 - 0xd000) * 8];
    uint8_t   mIo[0xff80 - 0xff00];
    uint8_t   mHRam[0xffff - 0xff80];
};

}  // namespace memory
