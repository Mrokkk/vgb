#pragma once

#include <cstddef>

#include "fwd.hpp"
#include "memory/generic.hpp"

namespace ppu
{

struct Video
{
    Video();
    ~Video();

    void start(const Config& config);
    void stop();
    void reset();

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
    void initPpu();
    void initRenderer();
    void drawLine();
    void renderFrame();
    void scheduleHsync(size_t cycles);

    IOImpl& getIo();

    bool mGraphical;
};

}  // namespace ppu
