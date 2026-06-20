#include "cartridge.hpp"

#include <cstdint>

#include "game_boy.hpp"
#include "memory/memory_map.hpp"

namespace memory
{

enum
{
    MEMORY_BANK_SIZE = 0x4000,
    RAM_BANK_SIZE    = 0x2000,
};

Cartridge::Cartridge()
    : mRamEnabled(false)
    , mRamDirty(false)
    , mAllocatedRam(false)
    , mMBC(MBC::NoMBC)
    , mRom(nullptr)
    , mRam(nullptr)
    , mRomSize(0)
    , mRamSize(0)
    , mRamBank(0)
    , mBanks(0)
    , mBank(0)
{
}

Cartridge::~Cartridge()
{
    if (mAllocatedRam)
    {
        delete [] mRam;
    }
}

static void copyTitle(const char* from, char* to)
{
    size_t i;
    for (i = 0; i < 16; ++i)
    {
        if (i > 0 and from[i] == ' ' and from[i - 1] == ' ')
        {
            i--;
            break;
        }
        to[i] = from[i];
    }
    to[i] = '\0';
}

void Cartridge::initialize(void* rom, void* ram)
{
    mRom     = static_cast<uint8_t*>(rom);
    mRomSize = romSize();
    mRamSize = ramSize();
    if (ram)
    {
        mRam = static_cast<uint8_t*>(ram);
    }
    else
    {
        mRam = mRamSize
            ? new uint8_t[mRamSize]
            : nullptr;
        mAllocatedRam = !!mRam;
    }
    mMBC     = MBC();
    mBanks   = (mRomSize + 1) / MEMORY_BANK_SIZE;
    copyTitle(mHeader->title, mTitle);
}

void Cartridge::reset()
{
    mBank    = 0;
    mRamBank = 0;
}

uint8_t Cartridge::load(uint16_t addr) const
{
    if (addr < MEMORY_BANK_SIZE)
    {
        return mRom[addr];
    }
    else
    {
        return mRom[MEMORY_BANK_SIZE * mBank + addr];
    }
}

void Cartridge::store(uint16_t addr, uint8_t value)
{
    constexpr static uint8_t bankMasks[] = {
        0x00, // No MBC
        0x1f, // MBC1
        0x00, // MBC2
        0x7f, // MBC3
    };

    switch (mMBC)
    {
        case MBC::NoMBC:
            return;

        case MBC::MBC1:
            if (addr < 0x2000)
            {
                mRamEnabled = value == 0xa;
                return;
            }
            else if (addr >= 0x2000 and addr < 0x4000)
            {
                mBank = value & bankMasks[static_cast<uint8_t>(mMBC)];
                if (mBank > 0) [[likely]]
                {
                    mBank--;
                }
                return;
            }
            break;

        case MBC::MBC3:
            if (addr < 0x2000)
            {
                mRamEnabled = value == 0xa;
                return;
            }
            else if (addr >= 0x2000 and addr < 0x4000)
            {
                mBank = value & bankMasks[static_cast<uint8_t>(mMBC)];
                if (mBank > 0) [[likely]]
                {
                    mBank--;
                }
                return;
            }
            else if (addr >= 0x4000)
            {
                if (value <= 0x07)
                {
                    mRamBank = value;
                }
                else if (value <= 0x0c)
                {
                    // TODO: add support for RTC register
                }
                return;
            }
        default:
            break;
    }

    gb.cpu.exc.reportSegmentationFault(addr, true, value);
}

uint8_t Cartridge::loadRam(uint16_t addr) const
{
    addr = Map::EXT_RAM.relative(addr);
    if (not mRam) [[unlikely]]
    {
        return 0xff;
    }
    return mRam[mRamBank * RAM_BANK_SIZE + addr];
}

void Cartridge::storeRam(uint16_t addr, uint8_t value)
{
    addr = Map::EXT_RAM.relative(addr);
    if (not mRam) [[unlikely]]
    {
        return;
    }
    mRam[mRamBank * RAM_BANK_SIZE + addr] = value;
    mRamDirty = true;
}

const char* Cartridge::getTitle() const
{
    return mTitle;
}

const char* Cartridge::getType() const
{
    switch (mHeader->type)
    {
        case CartridgeType::ROM_ONLY:
            return "ROM only";
        case CartridgeType::MBC1:
            return "MBC1";
        case CartridgeType::MBC1_RAM:
            return "MBC1+RAM";
        case CartridgeType::MBC1_RAM_BATTERY:
            return "MBC1+RAM+BATTERY";
        case CartridgeType::MBC2:
            return "MBC2";
        case CartridgeType::MBC2_BATTERY:
            return "MBC2+BATTERY";
        case CartridgeType::ROM_RAM:
            return "ROM+RAM";
        case CartridgeType::ROM_RAM_BATTERY:
            return "ROM+RAM+BATTERY";
        case CartridgeType::MMM01:
            return "MMM01";
        case CartridgeType::MMM01_RAM:
            return "MMM01+RAM";
        case CartridgeType::MMM01_RAM_BATTERY:
            return "MMM01+RAM+BATTERY";
        case CartridgeType::MBC3_TIMER_BATTERY:
            return "MBC3+TIMER+BATTERY";
        case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            return "MBC3+TIMER+RAM+BATTERY";
        case CartridgeType::MBC3:
            return "MBC3";
        case CartridgeType::MBC3_RAM:
            return "MBC3+RAM";
        case CartridgeType::MBC3_RAM_BATTERY:
            return "MBC3+RAM+BATTERY";
        case CartridgeType::MBC5:
            return "MBC5";
        case CartridgeType::MBC5_RAM:
            return "MBC5+RAM";
        case CartridgeType::MBC5_RAM_BATTERY:
            return "MBC5+RAM+BATTERY";
        case CartridgeType::MBC5_RUMBLE:
            return "MBC5+RUMBLE";
        case CartridgeType::MBC5_RUMBLE_RAM:
            return "MBC5+RUMBLE+RAM";
        case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            return "MBC5+RUMBLE+RAM+BATTERY";
        case CartridgeType::MBC6:
            return "MBC6";
        case CartridgeType::MBC7_SENSOR_RUMBLE_RAM_BATTERY:
            return "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
        case CartridgeType::POCKET_CAMERA:
            return "POCKET CAMERA";
        case CartridgeType::BANDAI_TAMA5:
            return "BANDAI TAMA5";
        case CartridgeType::HuC3:
            return "HuC3";
        case CartridgeType::HuC1_RAM_BATTERY:
            return "HuC1+RAM+BATTERY";
        default:
            return "unknown";
    }
}

MBC Cartridge::MBC() const
{
    switch (mHeader->type)
    {
        case CartridgeType::MBC1:
        case CartridgeType::MBC1_RAM:
        case CartridgeType::MBC1_RAM_BATTERY:
            return MBC::MBC1;

        case CartridgeType::MBC2:
        case CartridgeType::MBC2_BATTERY:
            return MBC::MBC2;

        case CartridgeType::MBC3:
        case CartridgeType::MBC3_RAM:
        case CartridgeType::MBC3_RAM_BATTERY:
        case CartridgeType::MBC3_TIMER_BATTERY:
        case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            return MBC::MBC3;

        case CartridgeType::MBC5:
        case CartridgeType::MBC5_RAM:
        case CartridgeType::MBC5_RAM_BATTERY:
        case CartridgeType::MBC5_RUMBLE:
        case CartridgeType::MBC5_RUMBLE_RAM:
        case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            return MBC::MBC5;

        case CartridgeType::MBC6:
            return MBC::MBC6;

        case CartridgeType::MBC7_SENSOR_RUMBLE_RAM_BATTERY:
            return MBC::MBC7;

        default:
            return MBC::NoMBC;
    }
}

}  // namespace memory
