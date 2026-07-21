#pragma once

#include <cstdint>

#include "fwd.hpp"
#include "sys/renderer.hpp"

#define GB_LCD_RESX 160
#define GB_LCD_RESY 144

void createPpu(GameBoy& gb);

uint32_t* getPalette(GameBoy& gb);
void setPalette(GameBoy& gb, uint32_t* palette);
sys::Texture renderMap(bool drawScxScyWindow);
