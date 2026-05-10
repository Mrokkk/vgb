#pragma once

#include <cstddef>

#include "memory/generic.hpp"

namespace apu
{

struct Sound
{
    Sound();
    ~Sound();

    void start();
    void stop();

    struct IO : memory::GenericIO<0xff40 - 0xff10>
    {
        void store(uint8_t addr, uint8_t value);
        uint8_t load(uint8_t addr) const;
    };

    IO   io;
};

}  // namespace apu
