#include "cartridge.hpp"

#include <cstring>

#include "game_boy.hpp"

namespace memory
{

enum
{
    MEMORY_BANK_SIZE = 0x4000,
};

Cartridge::Cartridge()
    : mHeader(nullptr)
    , mRamEnabled(false)
    , mMBC(MBC::NoMBC)
    , mSize(0)
    , mBank(0)
    , mRam(nullptr)
{
}

Cartridge::~Cartridge()
{
    if (mRam)
    {
        delete [] mRam;
    }
}

void Cartridge::initialize(const uint8_t* data)
{
    if (mRam)
    {
        delete [] mRam;
    }
    mHeader = reinterpret_cast<const CartridgeHeader*>(data);
    mSize = romSize();
    mRamSize = ramSize();
    mRam = allocateRam(mRamSize);
    mMBC = MBC();
    mBanks = (mSize + 1) / MEMORY_BANK_SIZE;
}

void Cartridge::reset()
{
    if (mRam)
    {
        memset(mRam, 0, mRamSize);
    }
    mBank = 0;
}

uint8_t Cartridge::load(uint16_t addr) const
{
    if (addr < MEMORY_BANK_SIZE)
    {
        return mData[addr];
    }
    else
    {
        return mData[MEMORY_BANK_SIZE * mBank + addr];
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
        case MBC::MBC1:
        case MBC::MBC3:
            if (addr < 0x2000)
            {
                mRamEnabled = value == 0xa;
                return;
            }
            else if (addr >= 0x2000 and addr < 0x4000)
            {
                mBank = value & bankMasks[(uint8_t)mMBC];
                if (mBank > 0) [[likely]]
                {
                    mBank--;
                }
                return;
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
    return mRam[addr];
}

void Cartridge::storeRam(uint16_t addr, uint8_t value)
{
    if (not mRam) [[unlikely]]
    {
        return;
    }
    mRam[addr] = value;
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

uint8_t* Cartridge::allocateRam(size_t size)
{
    return size
        ? new uint8_t[size]
        : nullptr;
}

//std::ostream& operator<<(std::ostream& os, CartridgeType type)
//{
    //switch (type)
    //{
        //case CartridgeType::ROM_ONLY:                       return os << "ROM only";
        //case CartridgeType::MBC1:                           return os << "MBC1";
        //case CartridgeType::MBC1_RAM:                       return os << "MBC1+RAM";
        //case CartridgeType::MBC1_RAM_BATTERY:               return os << "MBC1+RAM+BATTERY";
        //case CartridgeType::MBC2:                           return os << "MBC2";
        //case CartridgeType::MBC2_BATTERY:                   return os << "MBC2+BATTERY";
        //case CartridgeType::ROM_RAM:                        return os << "ROM+RAM";
        //case CartridgeType::ROM_RAM_BATTERY:                return os << "ROM+RAM+BATTERY";
        //case CartridgeType::MMM01:                          return os << "MMM01";
        //case CartridgeType::MMM01_RAM:                      return os << "MMM01+RAM";
        //case CartridgeType::MMM01_RAM_BATTERY:              return os << "MMM01+RAM+BATTERY";
        //case CartridgeType::MBC3_TIMER_BATTERY:             return os << "MBC3+TIMER+BATTERY";
        //case CartridgeType::MBC3_TIMER_RAM_BATTERY:         return os << "MBC3+TIMER+RAM+BATTERY";
        //case CartridgeType::MBC3:                           return os << "MBC3";
        //case CartridgeType::MBC3_RAM:                       return os << "MBC3+RAM";
        //case CartridgeType::MBC3_RAM_BATTERY:               return os << "MBC3+RAM+BATTERY";
        //case CartridgeType::MBC5:                           return os << "MBC5";
        //case CartridgeType::MBC5_RAM:                       return os << "MBC5+RAM";
        //case CartridgeType::MBC5_RAM_BATTERY:               return os << "MBC5+RAM+BATTERY";
        //case CartridgeType::MBC5_RUMBLE:                    return os << "MBC5+RUMBLE";
        //case CartridgeType::MBC5_RUMBLE_RAM:                return os << "MBC5+RUMBLE+RAM";
        //case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:        return os << "MBC5+RUMBLE+RAM+BATTERY";
        //case CartridgeType::MBC6:                           return os << "MBC6";
        //case CartridgeType::MBC7_SENSOR_RUMBLE_RAM_BATTERY: return os << "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
        //case CartridgeType::POCKET_CAMERA:                  return os << "POCKET CAMERA";
        //case CartridgeType::BANDAI_TAMA5:                   return os << "BANDAI TAMA5";
        //case CartridgeType::HuC3:                           return os << "HuC3";
        //case CartridgeType::HuC1_RAM_BATTERY:               return os << "HuC1+RAM+BATTERY";
    //}
    //return os << "Unknown(" << int(type) << ')';
//}

}  // namespace memory
