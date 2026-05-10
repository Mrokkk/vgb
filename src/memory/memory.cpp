#include "memory.hpp"

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

void Memory::loadCartridge(void* data)
{
    mCartridge.initialize(static_cast<uint8_t*>(data));
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

        GENERIC_LOAD(BASE_WRAM, mBaseWRam);
        GENERIC_LOAD(BANKED_WRAM, mSwitchableWRam);

        MEMORY_RANGE(OAM):
            return gb.vid.oam.load(addr - Map::OAM.start);

#define IO_LOAD(MEM, START, END) \
        if (addr >= START and addr <= END) \
        { \
            return MEM.load(addr - START); \
        }

        MEMORY_RANGE(IO):
            if (addr == 0xff05)
            {
                return gb.cpu.$if;
            }
            if (addr == 0xff00)
            {
                return 0xff;
            }
            IO_LOAD(gb.cpu.timer, 0xff04, 0xff07);
            IO_LOAD(gb.snd.io, 0xff10, 0xff3f);
            IO_LOAD(gb.vid.io, 0xff40, 0xff4b);
            break;

        GENERIC_LOAD(HRAM, mHRam);

        case 0xffff:
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

        MEMORY_RANGE(BASE_WRAM):
            return store(mBaseWRam, addr - Map::BASE_WRAM.start, val);

        GENERIC_STORE(BANKED_WRAM, mSwitchableWRam);

        MEMORY_RANGE(OAM):
            return gb.vid.oam.store(addr - Map::OAM.start, val);

#define IO_STORE(MEM, START, END) \
            if (addr >= START and addr <= END) \
            { \
                return MEM.store(addr - START, val); \
            }

        MEMORY_RANGE(IO):
            if (addr == 0xff00)
            {
                return;
            }
            if (addr == 0xff50 and val > 0)
            {
                mBootRomEnabled = false;
                return;
            }
            if (addr >= 0xff01 and addr <= 0xff02)
            {
                // Serial com, ignore
                return;
            }
            if (addr == 0xff0f)
            {
                gb.cpu.$if = val & 0x1f;
                return;
            }
            IO_STORE(gb.cpu.timer, 0xff04, 0xff07);
            IO_STORE(gb.snd.io, 0xff10, 0xff3f);
            IO_STORE(gb.vid.io, 0xff40, 0xff4b);
            break;

        GENERIC_STORE(HRAM, mHRam);

        case 0xffff:
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
