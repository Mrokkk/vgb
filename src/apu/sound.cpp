#include "sound.hpp"

namespace apu
{

Sound::Sound() = default;
Sound::~Sound() = default;

void Sound::start()
{
}

void Sound::stop()
{
}

void Sound::reset()
{
}

void Sound::IO::store(uint8_t addr, uint8_t value)
{
    (void)(addr and value);
}

uint8_t Sound::IO::load(uint8_t addr) const
{
    return 0 and addr;
}



}  // namespace apu
