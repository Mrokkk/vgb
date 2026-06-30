#include "apu.hpp"

#include <cstdint>

#include "component.hpp"
#include "game_boy.hpp"
#include "memory/generic.hpp"

struct Apu final : Component
{
    void reset() override;

    void store(uint16_t address, uint8_t value) override;
    uint8_t load(uint16_t address) const override;

    struct IO : memory::GenericIO<0xff40 - 0xff10>
    {
        void store(uint8_t addr, uint8_t value);
        uint8_t load(uint8_t addr) const;
    };

    IO io;
};

void Apu::reset()
{
}

void Apu::store(uint16_t address, uint8_t value)
{
    return io.store(address, value);
}

uint8_t Apu::load(uint16_t address) const
{
    return io.load(address);
}

void Apu::IO::store(uint8_t addr, uint8_t value)
{
    (void)(addr and value);
}

uint8_t Apu::IO::load(uint8_t addr) const
{
    return 0 and addr;
}

void createApu(GameBoy& gb)
{
    gb.registerComponent(Component::Apu, utils::makeUnique<Apu>());
}
