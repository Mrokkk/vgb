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
    0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e,
    0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
    0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9,
    0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20,
    0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
    0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2,
    0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06,
    0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
    0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17,
    0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
    0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
    0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
    0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
    0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50
};

Memory::Memory()
    : mBootRomEnabled(true)
{
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
                    return gb.components[Component::Joypad]->load(relative - 0x00);

                IO_MEMORY_RANGE(0x01, 0x03):
                    return gb.components[Component::Serial]->load(relative - 0x01);

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
