#include "memory.hpp"

#include <cstring>

#include <fmt/base.h>

#include "cpu/sm83.hpp"
#include "game_boy.hpp"
#include "memory/memory_map.hpp"
#include "utils/byte_order.hpp"
#include "utils/inline.hpp"

namespace memory
{

uint8_t dmgBootRom[0x100] = {
#ifndef __clang__
#embed "../../roms/dmg_boot.bin"
#endif
};

Memory::Memory()
    : mBootRomEnabled(true)
{
}

Memory::~Memory() = default;

ALWAYS_INLINE static uint8_t load(const uint8_t* arr, uint16_t addr)
{
    return arr[addr];
}

ALWAYS_INLINE static void store(uint8_t* arr, uint16_t addr, uint8_t data)
{
    arr[addr] = data;
}

void Memory::loadCartridge(const void* data)
{
    mCartridge.initialize(static_cast<const uint8_t*>(data));
}

void Memory::reset()
{
    mBootRomEnabled = true;
    mCartridge.reset();
    memset(mBaseWRam, 0, sizeof(mBaseWRam));
    memset(mSwitchableWRam, 0, sizeof(mSwitchableWRam));
    memset(mHRam, 0, sizeof(mHRam));
}

#define MEMORY_RANGE(RANGE) \
    case Map::RANGE.start ... Map::RANGE.end - 1

#define GENERIC_LOAD(RANGE, ARR) \
    MEMORY_RANGE(RANGE): \
        return load(ARR, addr - Map::RANGE.start)

#define GENERIC_STORE(RANGE, ARR) \
    MEMORY_RANGE(RANGE): \
        return store(ARR, addr - Map::RANGE.start, val)

uint8_t Memory::load8(uint16_t addr) const
{
    switch (addr)
    {
        MEMORY_RANGE(BOOT_ROM):
            if (mBootRomEnabled)
            {
                return load(dmgBootRom, addr);
            }
            [[fallthrough]];

        MEMORY_RANGE(ROM):
            return mCartridge.load(addr);

        MEMORY_RANGE(VRAM):
            return gb.vid.vram.load(addr - Map::VRAM.start);

        MEMORY_RANGE(EXT_RAM):
            return mCartridge.loadRam(addr - Map::EXT_RAM.start);

        GENERIC_LOAD(BASE_WRAM, mBaseWRam);
        GENERIC_LOAD(BANKED_WRAM, mSwitchableWRam);

        MEMORY_RANGE(OAM):
            return gb.vid.oam.load(addr - Map::OAM.start);

        MEMORY_RANGE(IO):
            return mIo.load(addr - Map::IO.start);

        GENERIC_LOAD(HRAM, mHRam);

        case Map::IE:
            return gb.cpu.ie;
    }

    gb.cpu.exc.reportSegmentationFault(addr, false);
    return 0;
}

uint16_t Memory::load16(uint16_t addr) const
{
    return utils::le16(load8(addr), load8(addr + 1));
}

void Memory::store8(uint16_t addr, uint8_t val)
{
    switch (addr)
    {
        MEMORY_RANGE(BOOT_ROM):
            if (mBootRomEnabled)
            {
                break;
            }
            [[fallthrough]];

        MEMORY_RANGE(ROM):
            return mCartridge.store(addr, val);

        MEMORY_RANGE(VRAM):
            return gb.vid.vram.store(addr - Map::VRAM.start, val);

        MEMORY_RANGE(EXT_RAM):
            return mCartridge.storeRam(addr - Map::EXT_RAM.start, val);

        MEMORY_RANGE(BASE_WRAM):
            return store(mBaseWRam, addr - Map::BASE_WRAM.start, val);

        GENERIC_STORE(BANKED_WRAM, mSwitchableWRam);

        MEMORY_RANGE(OAM):
            return gb.vid.oam.store(addr - Map::OAM.start, val);

        MEMORY_RANGE(IO):
            if (addr == 0xff50 and val > 0)
            {
                mBootRomEnabled = false;
                return;
            }
            return mIo.store(addr - Map::IO.start, val);

        GENERIC_STORE(HRAM, mHRam);

        case Map::IE:
            gb.cpu.ie = val & 0x1f;
            return;
    }

    gb.cpu.exc.reportSegmentationFault(addr, true);
}

void Memory::store16(uint16_t addr, uint16_t val)
{
    store8(addr, utils::lsb(val));
    store8(addr + 1, utils::msb(val));
}

}  // namespace memory
