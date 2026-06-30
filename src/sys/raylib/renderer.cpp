#define LOG_HEADER "RaylibRenderer"
#include "renderer.hpp"

#include <cstdint>
#include <cstdio>

#include <raylib.h>
#include <rlImGui.h>

#include "core/logger.hpp"
#include "debugger/main.hpp"
#include "game_boy.hpp"
#include "ppu.hpp"
#include "save_serializer.hpp"
#include "sys/platform.hpp"
#include "sys/renderer.hpp"
#include "utils/inline.hpp"
#include "utils/unique_ptr.hpp"

namespace sys::raylib
{

#define RAYLIB_LOG 1
#define DEFAULT_SCALE 6

struct RaylibRenderer final : Renderer
{
    RaylibRenderer();
    ~RaylibRenderer();

    void render() override;
    void drawPixel(uint8_t x, uint8_t y, uint16_t color) override;
    unsigned int renderMap(bool drawWindow) override;
    ALWAYS_INLINE void drawWindow();
    ALWAYS_INLINE void drawRectangle(float x, float y, float width, float height);

private:
    Image         mScreenImage;
    Texture2D     mScreenTexture;
    Image         mMapImage;
    Texture2D     mMapTexture;

    static constexpr Color colors[] = {
        {0xd0, 0xd0, 0xd0, 0xff},
        {0x80, 0x80, 0x80, 0xff},
        {0x50, 0x50, 0x50, 0xff},
        {0x00, 0x00, 0x00, 0xff},
    };
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

RaylibRenderer::RaylibRenderer()
{
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_RESIZABLE);
    SetTraceLogCallback(raylibLogFormat);
    SetTargetFPS(60);
    InitWindow(GB_LCD_RESX * DEFAULT_SCALE, GB_LCD_RESY * DEFAULT_SCALE, "GameBoy");
    SetWindowState(FLAG_WINDOW_MAXIMIZED | FLAG_WINDOW_RESIZABLE);

    mScreenImage = GenImageColor(GB_LCD_RESX, GB_LCD_RESY, DARKGRAY);
    mScreenTexture = LoadTextureFromImage(mScreenImage);

    if (gb.config.useDebugger)
    {
        mMapImage = GenImageColor(256, 256, DARKGRAY);
        mMapTexture = LoadTextureFromImage(mMapImage);
        rlImGuiSetup(true);
    }

    SaveSerializer::registerData(mScreenImage.data, mScreenImage.height * mScreenImage.width * 4);
}

RaylibRenderer::~RaylibRenderer()
{
    CloseWindow();
}

#define RENDER() \
    for (int i = (BeginDrawing(), 0); i == 0; i = (EndDrawing(), 1))

#define IMGUI() \
    for (int i = (rlImGuiBegin(), 0); i == 0; i = (rlImGuiEnd(), 1))

void RaylibRenderer::render()
{
    if (WindowShouldClose()) [[unlikely]]
    {
        gb.cpu.exc.reportUserInterruption();
        return;
    }

    UpdateTexture(mScreenTexture, mScreenImage.data);

    RENDER()
    {
        ClearBackground(DARKGRAY);

        if (gb.config.useDebugger)
        {
            IMGUI()
            {
                debugger::frame(mScreenTexture.id);
            }
        }
        else
        {
            auto resX = GetScreenWidth();
            auto resY = GetScreenHeight();
            auto scaleX = float(resX) / GB_LCD_RESX;
            auto scaleY = float(resY) / GB_LCD_RESY;
            auto scale = std::min(scaleX, scaleY);
            auto posX = (resX - scale * GB_LCD_RESX) / 2;
            auto posY = (resY - scale * GB_LCD_RESY) / 2;
            DrawTextureEx(mScreenTexture, Vector2{posX, posY}, 0.0f, std::min(scaleX, scaleY), WHITE);
            DrawFPS(resX - 100, 20);
        }
    }
}

void RaylibRenderer::drawPixel(uint8_t x, uint8_t y, uint16_t color)
{
    ImageDrawPixel(&mScreenImage, x, y, colors[color]);
}

ALWAYS_INLINE void RaylibRenderer::drawRectangle(float x, float y, float width, float height)
{
    ImageDrawRectangleLines(&mMapImage, Rectangle{.x = x, .y = y, .width = width, .height = height}, 1, RED);
}

ALWAYS_INLINE void RaylibRenderer::drawWindow()
{
    int scy = gb.cpu.mem.load8(0xff42);
    int scx = gb.cpu.mem.load8(0xff43);

    drawRectangle(scx, scy, GB_LCD_RESX, GB_LCD_RESY);

    if (scx + GB_LCD_RESX >= 256 and scy + GB_LCD_RESY >= 256)
    {
        drawRectangle(scx - 256, scy - 256, GB_LCD_RESX, GB_LCD_RESY);
    }
    if (scx + GB_LCD_RESX >= 256)
    {
        drawRectangle(scx - 256, scy, GB_LCD_RESX, GB_LCD_RESY);
    }
    if (scy + GB_LCD_RESY >= 256)
    {
        drawRectangle(scx, scy - 256, GB_LCD_RESX, GB_LCD_RESY);
    }
}

unsigned int RaylibRenderer::renderMap(bool drawScxScyWindow)
{
    const auto lcdc = gb.cpu.mem.load8(0xff40);

    const auto tileDataAddr = lcdc & (1 << 4)
        ? 0x8000
        : 0x8800;

    const auto tileMapAddr = lcdc & (1 << 3)
        ? 0x9c00
        : 0x9800;

    for (size_t tileY = 0; tileY < 32; ++tileY)
    {
        for (size_t tileX = 0; tileX < 32; ++tileX)
        {
            const auto tileId = lcdc & (1 << 4)
                ? gb.cpu.mem.load8(tileMapAddr + tileY * 32 + tileX)
                : 128 + (int8_t)gb.cpu.mem.load8(tileMapAddr + tileY * 32 + tileX);

            for (uint8_t y = 0; y < 8; ++y)
            {
                const auto byte1 = gb.cpu.mem.load8(tileDataAddr + tileId * 16 + y * 2);
                const auto byte2 = gb.cpu.mem.load8(tileDataAddr + tileId * 16 + y * 2 + 1);

                for (uint8_t x = 0; x < 8; ++x)
                {
                    const auto color
                        = (((byte1 >> (7 - x)) & 1))
                        | (((byte2 >> (7 - x)) & 1) << 1);
                    ImageDrawPixel(&mMapImage, x + tileX * 8, y + tileY * 8, colors[color]);
                }
            }
        }
    }

    if (drawScxScyWindow)
    {
        drawWindow();
    }

    UpdateTexture(mMapTexture, mMapImage.data);

    return mMapTexture.id;
}

void createRenderer()
{
    platform.renderer = utils::makeUnique<RaylibRenderer>();
}

}  // namespace sys::raylib
