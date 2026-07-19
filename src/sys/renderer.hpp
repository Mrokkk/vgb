#pragma once

#include <cstdint>

#include "utils/unique_ptr.hpp"

namespace sys
{

struct Renderer
{
    virtual ~Renderer() = default;
    virtual void render() = 0;
    virtual void drawPixel(uint8_t x, uint8_t y, uint16_t color) = 0;
    virtual unsigned int renderMap(bool drawWindow) = 0;
    virtual uint32_t* getPalette() = 0;
    virtual void setPalette(uint32_t* palette) = 0;
};

using RendererPtr = utils::UniquePtr<Renderer>;

}  // namespace sys
