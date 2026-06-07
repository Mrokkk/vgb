#include "cartridge.hpp"

#include "game_boy.hpp"

namespace memory
{

enum
{
    MEMORY_BANK_SIZE = 0x4000,
    RAM_BANK_SIZE    = 0x2000,
};

Cartridge::Cartridge()
    : mRamEnabled(false)
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

void Cartridge::initialize(const void* rom, void* ram)
{
    mRom     = static_cast<const uint8_t*>(rom);
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
                if (value <= 7)
                {
                    mRamBank = value;
                    return;
                }
            }
        default:
            break;
    }

    gb.cpu.exc.reportSegmentationFault(addr, true);
}

uint8_t Cartridge::loadRam(uint16_t addr) const
{
    if (not mRam) [[unlikely]]
    {
        return 0xff;
    }
    return mRam[mRamBank * RAM_BANK_SIZE + addr];
}

void Cartridge::storeRam(uint16_t addr, uint8_t value)
{
    if (not mRam) [[unlikely]]
    {
        return;
    }
    mRam[mRamBank * RAM_BANK_SIZE + addr] = value;
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
