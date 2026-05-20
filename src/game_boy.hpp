#pragma once

#include "apu/sound.hpp"
#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "fwd.hpp"
#include "ppu/video.hpp"

struct GameBoy
{
    void run(const void* cartridge, const Config& config);
    void reset();
    void skipBootRom();

    cpu::SM83   cpu;
    ppu::Video  vid;
    apu::Sound  snd;
    EventSystem events;
private:
    bool mSkipBootRom;
};

extern GameBoy gb;
