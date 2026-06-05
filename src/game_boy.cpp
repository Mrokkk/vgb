#include "game_boy.hpp"

#include <fmt/base.h>

#include "config.hpp"
#include "cpu/printers.hpp"

void GameBoy::start(const void* rom, void* ram, const Config& config)
{
    cartridge.initialize(rom, ram);

    vid.start(config);
    inp.start(config);

    cpu.timer.start();

    if (config.skipBootRom)
    {
        skipBootRom();
    }
}

void GameBoy::run()
{
    auto result = cpu.run();
    if (not result) [[unlikely]]
    {
        fmt::println("Exception raised: {}", result.error());
    }
}

void GameBoy::reset()
{
    events.reset();
    cpu.reset();
    cartridge.reset();
    vid.reset();
    snd.reset();
    inp.reset();
    if (mSkipBootRom)
    {
        skipBootRom();
    }
}

void GameBoy::skipBootRom()
{
    mSkipBootRom = true;

    cpu.af  = 0x01b0;
    cpu.bc  = 0x0013;
    cpu.de  = 0x00d8;
    cpu.hl  = 0x014d;
    cpu.sp  = 0xfffe;
    cpu.pc  = 0x100;
    cpu.ie  = 0x00;
    cpu.$if = 0x00;
    cpu.ime = 0x0;

    cpu.mem.store8(0xff50, 0x01); // Disable boot ROM
    cpu.mem.store8(0xff40, 0x91); // Enable video
    cpu.mem.store8(0xff41, 0x85);
}
