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
};

using RendererPtr = utils::UniquePtr<Renderer>;

}  // namespace sys
