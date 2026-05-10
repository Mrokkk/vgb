#pragma once

#include <cstddef>

#include "memory/generic.hpp"

namespace ppu
{

struct Video
{
    Video();
    ~Video();

    void start();
    void stop();

    using BaseIO = memory::GenericIO<0xff4c - 0xff40>;

    struct IO : BaseIO
    {
        void store(uint8_t addr, uint8_t value);
        uint8_t load(uint8_t addr) const;
    };

    using VRAM = memory::GenericRAM<0x2000>;
    using OAM = memory::GenericRAM<0xa0>;

    IO   io;
    OAM  oam;
    VRAM vram;

private:
    struct IOImpl;
    void renderFrame();
    void scheduleHsync(size_t cycles);

    IOImpl& getIo();
};

}  // namespace ppu
