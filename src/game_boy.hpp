#pragma once

#include "apu/sound.hpp"
#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "fwd.hpp"
#include "input/input.hpp"
#include "memory/cartridge.hpp"
#include "ppu/video.hpp"

struct GameBoy
{
    void start(const void* cartridge, void* ram, const Config& config);
    void run();
    void reset();
    void skipBootRom();

    cpu::SM83         cpu;
    ppu::Video        vid;
    input::Input      inp;
    apu::Sound        snd;
    memory::Cartridge cartridge;
    EventSystem       events;

private:
    bool mSkipBootRom;
};

extern GameBoy gb;
