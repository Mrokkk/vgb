#pragma once

#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "ppu/video.hpp"

struct GameBoy
{
    cpu::SM83   cpu;
    ppu::Video  vid;
    EventSystem events;
};

extern GameBoy gb;
