#pragma once

#include <cstddef>
#include <cstdint>

#include "utils/units.hpp"

namespace memory
{

enum class CartridgeType : uint8_t
{
    ROM_ONLY                        =  0x00,
    MBC1                            =  0x01,
    MBC1_RAM                        =  0x02,
    MBC1_RAM_BATTERY                =  0x03,
    MBC2                            =  0x05,
    MBC2_BATTERY                    =  0x06,
    ROM_RAM                         =  0x08,
    ROM_RAM_BATTERY                 =  0x09,
    MMM01                           =  0x0b,
    MMM01_RAM                       =  0x0c,
    MMM01_RAM_BATTERY               =  0x0d,
    MBC3_TIMER_BATTERY              =  0x0f,
    MBC3_TIMER_RAM_BATTERY          =  0x10,
    MBC3                            =  0x11,
    MBC3_RAM                        =  0x12,
    MBC3_RAM_BATTERY                =  0x13,
    MBC5                            =  0x19,
    MBC5_RAM                        =  0x1a,
    MBC5_RAM_BATTERY                =  0x1b,
    MBC5_RUMBLE                     =  0x1c,
    MBC5_RUMBLE_RAM                 =  0x1d,
    MBC5_RUMBLE_RAM_BATTERY         =  0x1e,
    MBC6                            =  0x20,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY  =  0x22,
    POCKET_CAMERA                   =  0xfc,
    BANDAI_TAMA5                    =  0xfd,
    HuC3                            =  0xfe,
    HuC1_RAM_BATTERY                =  0xff,
};

enum class MBC : uint8_t
{
    NoMBC,
    MBC1 = 1,
    MBC2 = 2,
    MBC3 = 3,
    MBC5 = 5,
    MBC6 = 6,
    MBC7 = 7,
};

struct [[gnu::packed]] CartridgeHeader
{
    uint8_t       reserved[0x100];
    uint8_t       entryPoint[4];
    uint8_t       nintendoLogo[48];
    char          title[16];
    uint16_t      licenseCode;
    uint8_t       sgbFlag;
    CartridgeType type;
    uint8_t       romSize;
    uint8_t       ramSize;
    uint8_t       destinationCode;
    uint8_t       oldLicenseCode;
    uint8_t       maskRomVersion;
    uint8_t       headerChecksum;
    uint16_t      globalChecksum;
};

struct Cartridge
{
    Cartridge();
    ~Cartridge();

    void initialize(uint8_t* data);

    uint8_t load(uint16_t addr) const;
    void store(uint16_t addr, uint8_t value);

    uint32_t romSize() const
    {
        return 32 * KiB * (1 << mHeader->romSize);
    }

    uint32_t ramSize() const
    {
        switch (mHeader->ramSize)
        {
            case 2: return 8 * KiB;
            case 3: return 32 * KiB;
            case 4: return 128 * KiB;
            case 5: return 64 * KiB;
        }
        return 0;
    }

    enum MBC MBC() const;

private:
    static uint8_t* allocateRam(size_t size);

    union
    {
        CartridgeHeader* mHeader;
        uint8_t*         mData;
    };
    bool     mRamEnabled;
    enum MBC mMBC;
    size_t   mSize;
    uint32_t mBanks;
    uint32_t mBank;
    uint8_t* mRam;
};

}  // namespace memory
