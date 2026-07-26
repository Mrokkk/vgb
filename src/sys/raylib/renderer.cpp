#define LOG_HEADER "RaylibRenderer"
#include "renderer.hpp"

#include <cstdint>
#include <cstdio>

#include <raylib.h>
#include <rlImGui.h>

#include "config.hpp"
#include "core/logger.hpp"
#include "game_boy.hpp"
#include "ppu.hpp"
#include "sys/platform.hpp"
#include "sys/renderer.hpp"
#include "utils/unique_ptr.hpp"

namespace sys::raylib
{

#define RAYLIB_LOG 1
#define DEFAULT_SCALE 6

struct ImageAndTexture final
{
    bool      dirty;
    void*     pixels;
    Texture2D texture;
};

struct RaylibRenderer final : Renderer
{
    RaylibRenderer(const Config& config);
    ~RaylibRenderer();

    void beginRendering() override;
    void endRendering() override;

    Texture createTexture(int resX, int resY, void* pixels) override;

private:
    std::vector<ImageAndTexture> mTextures;
};

static void raylibLogFormat(int msgType, const char* text, va_list args)
{
    if (not RAYLIB_LOG)
    {
        return;
    }

    char buf[256];
    core::Severity severity;

    switch (msgType)
    {
        case LOG_INFO:    severity = core::Severity::info; break;
        case LOG_ERROR:   severity = core::Severity::error; break;
        case LOG_WARNING: severity = core::Severity::warning; break;
        default:          severity = core::Severity::debug; break;
    }

    auto len = vsnprintf(buf, sizeof(buf), text, args);
    buf[len] = 0;

    core::logger.log(severity).buffer().assign(buf, len);
}

RaylibRenderer::RaylibRenderer(const Config&)
{
    mTextures.reserve(128);
    SetTraceLogCallback(raylibLogFormat);

    SetConfigFlags(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(GB_LCD_RESX * DEFAULT_SCALE, GB_LCD_RESY * DEFAULT_SCALE, "GameBoy");
    SetWindowState(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_RESIZABLE);
    rlImGuiSetup(true);
}

RaylibRenderer::~RaylibRenderer()
{
    for (auto& e : mTextures)
    {
        UnloadTexture(e.texture);
    }
    rlImGuiShutdown();
    CloseWindow();
}

void RaylibRenderer::beginRendering()
{
    for (auto& e : mTextures)
    {
        if (e.dirty)
        {
            UpdateTexture(e.texture, e.pixels);
            e.dirty = false;
        }
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    rlImGuiBegin();

    if (WindowShouldClose()) [[unlikely]]
    {
        gb.cpu.exc.reportUserInterruption();
    }
}

void RaylibRenderer::endRendering()
{
    rlImGuiEnd();
    EndDrawing();
}

Texture RaylibRenderer::createTexture(int resX, int resY, void* pixels)
{
    Image image = {
        .data = pixels,
        .width = resX,
        .height = resY,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    auto texture = LoadTextureFromImage(image);
    int index = mTextures.size();

    auto& t = mTextures.emplace_back(ImageAndTexture{
        .dirty = false,
        .pixels = pixels,
        .texture = texture
    });

    return Texture{
        .index = index,
        .backendId = texture.id,
        .pixels = static_cast<uint32_t*>(image.data),
        .dirty = &t.dirty,
    };
}

void createRenderer(const Config& config)
{
    platform.renderer = utils::makeUnique<RaylibRenderer>(config);
}

}  // namespace sys::raylib
