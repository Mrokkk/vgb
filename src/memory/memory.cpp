#include "memory.hpp"

#include <cstring>

#include "cpu/sm83.hpp"
#include "game_boy.hpp"
#include "memory/memory_map.hpp"
#include "save_serializer.hpp"
#include "utils/byte_order.hpp"

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
    vram.bank = 0;
    bankedWorkRam.bank = 0;
    SaveSerializer::registerData("memory.vram", vram);
    SaveSerializer::registerData("memory.baseWorkRam", baseWorkRam);
    SaveSerializer::registerData("memory.bankedWorkRam", bankedWorkRam);
    SaveSerializer::registerData("memory.oam", oam);
    SaveSerializer::registerData("memory.highRam", highRam);
    SaveSerializer::registerData("memory.mBootRomEnabled", mBootRomEnabled);
}

Memory::~Memory() = default;

void Memory::reset()
{
    mBootRomEnabled = true;
    vram.bank = 0;
    memset(vram.data, 0, sizeof(vram.data));
    memset(baseWorkRam.data, 0, sizeof(baseWorkRam.data));
    bankedWorkRam.bank = 0;
    memset(bankedWorkRam.data, 0, sizeof(bankedWorkRam.data));
    memset(oam.data, 0, sizeof(oam.data));
    memset(highRam.data, 0, sizeof(highRam.data));
}

#define MEMORY_RANGE(RANGE) \
    case Map::RANGE.start ... Map::RANGE.end - 1

#define IO_MEMORY_RANGE(START, END) \
    case START ... END - 1

uint8_t Memory::load8(uint16_t addr) const
{
    switch (addr)
    {
        MEMORY_RANGE(BOOT_ROM):
            if (mBootRomEnabled)
            {
                return dmgBootRom[addr];
            }
            [[fallthrough]];

        MEMORY_RANGE(ROM):
            return gb.cartridge.load(addr);

        MEMORY_RANGE(VRAM):
            return vram.load(addr);

        MEMORY_RANGE(EXT_RAM):
            return gb.cartridge.loadRam(addr);

        MEMORY_RANGE(BASE_WRAM):
            return baseWorkRam.load(addr);

        MEMORY_RANGE(BANKED_WRAM):
            return bankedWorkRam.load(addr);

        MEMORY_RANGE(MIRROR):
            return 0xff;

        MEMORY_RANGE(OAM):
            return oam.load(addr);

        MEMORY_RANGE(INVALID):
            return 0xff;

        MEMORY_RANGE(IO):
        {
            auto relative = memory::Map::IO.relative(addr);

            switch (relative)
            {
                IO_MEMORY_RANGE(0x00, 0x01):
                    return gb.components[Component::Joypad]->load(relative);

                IO_MEMORY_RANGE(0x01, 0x03):
                    return gb.components[Component::Serial]->load(relative);

                IO_MEMORY_RANGE(0x04, 0x08):
                    return gb.components[Component::Timer]->load(relative - 0x04);

                IO_MEMORY_RANGE(0x0f, 0x10):
                    return gb.cpu.$if;

                IO_MEMORY_RANGE(0x10, 0x40):
                    return gb.components[Component::Apu]->load(relative - 0x10);

                IO_MEMORY_RANGE(0x40, 0x4c):
                    return gb.components[Component::Ppu]->load(relative - 0x40);

                IO_MEMORY_RANGE(0x4d, 0x4e):
                    return 0xff;

                IO_MEMORY_RANGE(0x50, 0x51):
                    return not mBootRomEnabled;
            }
            return 0xff;
        }

        MEMORY_RANGE(HRAM):
            return highRam.load(addr);

        MEMORY_RANGE(IE):
            return gb.cpu.ie;
    }

    gb.cpu.exc.reportSegmentationFault(addr, false);
    return 0xff;
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
            return gb.cartridge.store(addr, val);

        MEMORY_RANGE(VRAM):
            return vram.store(addr, val);

        MEMORY_RANGE(EXT_RAM):
            return gb.cartridge.storeRam(addr, val);

        MEMORY_RANGE(BASE_WRAM):
            return baseWorkRam.store(addr, val);

        MEMORY_RANGE(BANKED_WRAM):
            return bankedWorkRam.store(addr, val);

        MEMORY_RANGE(MIRROR):
            return;

        MEMORY_RANGE(OAM):
            return oam.store(addr, val);

        MEMORY_RANGE(INVALID):
            return;

        MEMORY_RANGE(IO):
        {
            auto relative = memory::Map::IO.relative(addr);

            switch (relative)
            {
                IO_MEMORY_RANGE(0x00, 0x01):
                    return gb.components[Component::Joypad]->store(relative, val);

                IO_MEMORY_RANGE(0x01, 0x03):
                    return gb.components[Component::Serial]->store(relative, val);

                IO_MEMORY_RANGE(0x04, 0x08):
                    return gb.components[Component::Timer]->store(relative - 0x04, val);

                IO_MEMORY_RANGE(0x0f, 0x10):
                    gb.cpu.$if = val & 0x1f;
                    return;

                IO_MEMORY_RANGE(0x10, 0x40):
                    return gb.components[Component::Apu]->store(relative - 0x10, val);

                IO_MEMORY_RANGE(0x40, 0x4c):
                    return gb.components[Component::Ppu]->store(relative - 0x40, val);

                IO_MEMORY_RANGE(0x4d, 0x4e):
                    return;

                IO_MEMORY_RANGE(0x50, 0x51):
                    if (val > 0)
                    {
                        mBootRomEnabled = false;
                    }
                    return;
            }
            return;
        }

        MEMORY_RANGE(HRAM):
            return highRam.store(addr, val);

        MEMORY_RANGE(IE):
            gb.cpu.ie = val & 0x1f;
            return;
    }

    gb.cpu.exc.reportSegmentationFault(addr, true, val);
}

uint16_t Memory::load16(uint16_t addr) const
{
    return utils::le16(load8(addr), load8(addr + 1));
}

void Memory::store16(uint16_t addr, uint16_t val)
{
    store8(addr, utils::lsb(val));
    store8(addr + 1, utils::msb(val));
}

}  // namespace memory
