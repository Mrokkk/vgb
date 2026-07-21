#pragma once

#include <cstdint>

#include "utils/unique_ptr.hpp"

namespace sys
{

using TextureId = unsigned long;

struct Texture
{
    int       index;
    TextureId backendId;
    uint32_t* pixels;
};

struct Renderer
{
    virtual ~Renderer() = default;
    virtual Texture createTexture(int resX, int resY, void* pixels) = 0;
    virtual void beginRendering() = 0;
    virtual void endRendering() = 0;
};

using RendererPtr = utils::UniquePtr<Renderer>;

}  // namespace sys
