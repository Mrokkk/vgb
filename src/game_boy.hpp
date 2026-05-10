#pragma once

#include "apu/sound.hpp"
#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "ppu/video.hpp"

struct GameBoy
{
    cpu::SM83   cpu;
    ppu::Video  vid;
    apu::Sound  snd;
    EventSystem events;
};

extern GameBoy gb;
