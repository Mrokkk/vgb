#include "io.hpp"

#include "game_boy.hpp"

namespace memory
{


#define IO_STORE(MEM, START, END) \
    if (addr >= START and addr <= END) \
    { \
        return MEM.store(addr - START, val); \
    }

#define IO_LOAD(MEM, START, END) \
    if (addr >= START and addr <= END) \
    { \
        return MEM.load(addr - START); \
    }

void IO::store(uint16_t addr, uint8_t val)
{
    if (addr == 0x00)
    {
        return gb.inp.store(val);
    }
    if (addr == 0x4d)
    {
        return;
    }
    if (addr >= 0x01 and addr <= 0x02)
    {
        // Serial com, ignore
        return;
    }
    if (addr == 0x0f)
    {
        gb.cpu.$if = val & 0x1f;
        return;
    }
    IO_STORE(gb.cpu.timer, 0x04, 0x07);
    IO_STORE(gb.snd.io, 0x10, 0x3f);
    IO_STORE(gb.vid.io, 0x40, 0x4b);
}

uint8_t IO::load(uint16_t addr) const
{
    if (addr == 0x0f)
    {
        return gb.cpu.$if;
    }
    if (addr == 0x00)
    {
        return gb.inp.load();
    }
    if (addr == 0x4d)
    {
        return 0xff;
    }

    IO_LOAD(gb.cpu.timer, 0x04, 0x07);
    IO_LOAD(gb.snd.io, 0x10, 0x3f);
    IO_LOAD(gb.vid.io, 0x40, 0x4b);

    return 0xff;
}

}  // namespace memory
