#include "video.hpp"

#include <cstdio>
#include <cstring>

#include <fmt/base.h>
#include <raylib.h>

#include "config.hpp"
#include "cpu/sm83.hpp"
#include "event.hpp"
#include "game_boy.hpp"
#include "sys/system.hpp"
#include "utils/inline.hpp"

#define RAYLIB_LOG 0

namespace ppu
{

static void raylibLogFormat(int msgType, const char* text, va_list args)
{
    if (not RAYLIB_LOG)
    {
        return;
    }

    fprintf(stderr, "[Raylib] ");

    switch (msgType)
    {
        case LOG_INFO:    fprintf(stderr, "[INF] "); break;
        case LOG_ERROR:   fprintf(stderr, "[ERR] "); break;
        case LOG_WARNING: fprintf(stderr, "[WRN] "); break;
        case LOG_DEBUG:   fprintf(stderr, "[DBG] "); break;
        default:          fprintf(stderr, "[???] "); break;
    }

    vfprintf(stderr, text, args);
    fputc('\n', stderr);
}

enum
{
    GB_LCD_RESX       = 160,
    GB_LCD_RESY       = 144,

    GB_TILE_RESX      = 8,
    GB_TILE_RESY      = 8,
    GB_TILE_BYTES     = 16,

    GB_TILE_DATA_RESX = 256,
    GB_TILE_DATA_RESY = 256,
    SCALING           = 5,

    FRAME_DURATION    = 70224,
    HSYNC_DURATION    = 456,
    VSYNC_LY_START    = 144,
    VSYNC_LY_END      = 154,
};

template <typename T, T MAX>
struct Counter
{
    ALWAYS_INLINE constexpr operator T() const
    {
        return mValue;
    }

    ALWAYS_INLINE constexpr T operator++(int)
    {
        const auto prev = mValue;
        if (++mValue >= MAX)
        {
            mValue = 0;
        }
        return prev;
    }

    ALWAYS_INLINE constexpr T operator++()
    {
        if (++mValue >= MAX)
        {
            mValue = 0;
        }
        return mValue;
    }

    ALWAYS_INLINE constexpr T operator=(T value)
    {
        return mValue = value;
    }

    ALWAYS_INLINE T get() const
    {
        return mValue;
    }

private:
    T mValue;
};

#define DEFINE_REGISTER(NAME, BODY) \
    union NAME \
    { \
        struct BODY; \
        uint8_t value; \
    }

DEFINE_REGISTER(LCDC,
{
    uint8_t bgWindowEnable:1;
    uint8_t objEnable:1;
    uint8_t objSize:1;
    uint8_t bgTileMapArea:1;
    uint8_t bgWindowDataArea:1;
    uint8_t windowEnable:1;
    uint8_t windowTileMapArea:1;
    uint8_t lcdEnable:1;
});

DEFINE_REGISTER(STAT,
{
    uint8_t ppuMode:2;
    uint8_t lycEqLy:1;
    uint8_t intMode0:1;
    uint8_t intMode1:1;
    uint8_t intMode2:1;
    uint8_t intLyc:1;
    uint8_t reserved:1;
});

using LY = Counter<uint8_t, VSYNC_LY_END>;

struct Video::IOImpl
{
    LCDC    lcdc;
    STAT    stat;
    uint8_t scy;
    uint8_t scx;
    LY      ly;
    uint8_t lyc;
    uint8_t dma;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;
};

Color colors[4] = {
    {0xff, 0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0xff},
    {0x00, 0x00, 0x00, 0xff},
    {0x00, 0x00, 0x00, 0xff},
};

Video::Video() = default;
Video::~Video() = default;

static Image screenImage;
static Texture2D screenTexture;

static Event hsync = Event::repeating({
    .prio = 0,
    .period = HSYNC_DURATION,
});

static Event dma = Event::oneShot({
    .prio = 0,
});

#define RENDER() \
    for (int i = (BeginDrawing(), 0); i == 0; i = (EndDrawing(), 1))

ALWAYS_INLINE Video::IOImpl& Video::getIo()
{
    return *reinterpret_cast<Video::IOImpl*>(io.data);
}

void Video::start(const Config& config)
{
    init();

    if (config.videoConfig == VideoConfig::Graphical)
    {
        mGraphical = true;
        SetTraceLogCallback(raylibLogFormat);
        InitWindow(GB_LCD_RESX * SCALING, GB_LCD_RESY * SCALING, "GameBoy");

        SetTargetFPS(60);

        screenImage = GenImageColor(GB_LCD_RESX, GB_LCD_RESY, WHITE);
        screenTexture = LoadTextureFromImage(screenImage);

        RENDER()
        {
            ClearBackground(BLACK);
        }
    }
}

void Video::stop()
{
    UnloadTexture(screenTexture);
    UnloadImage(screenImage);
    CloseWindow();
}

void Video::reset()
{
    memset(io.data, 0, sizeof(io.data));
    memset(oam.data, 0, sizeof(oam.data));
    memset(vram.data, 0, sizeof(vram.data));
    init();
}

void Video::init()
{
    auto& ioRo = *reinterpret_cast<IOImpl*>(io.roMasks);

    ioRo.stat.ppuMode = 3;
    ioRo.stat.lycEqLy = 1;
    ioRo.stat.reserved = 1;
    ioRo.ly = 0xff;

    auto& io = getIo();
    io.ly = VSYNC_LY_START - 1;

    hsync.setCallback(
        [this](size_t)
        {
            auto& io = getIo();
            if (++io.ly == VSYNC_LY_START)
            {
                if (mGraphical)
                {
                    renderFrame();
                }
                gb.cpu.raiseIrq(cpu::IRQ::VBlank);
                io.stat.ppuMode = 1;
            }
            else
            {
                io.stat.ppuMode = 3;
            }
        });

    dma.setCallback(
        [this](size_t)
        {
            auto& io = getIo();
            uint16_t src = ((uint16_t)io.dma << 8);

            for (uint16_t i = 0; i < 0xa0; ++i)
            {
                oam.data[i] = gb.cpu.mem.load8(src + i);
            }
        });

    gb.events.scheduleEvent(hsync, HSYNC_DURATION);
}

void Video::renderFrame()
{
    auto& io = getIo();

    sys::pingSupervision();

    if (WindowShouldClose()) [[unlikely]]
    {
        gb.cpu.exc.reportUserInterruption();
        return;
    }

    if (not io.lcdc.lcdEnable) [[unlikely]]
    {
        RENDER()
        {
            ClearBackground(WHITE);
            DrawFPS(SCALING * GB_LCD_RESX - 100, 20);
        }

        return;
    }

    const auto bgWindowDataOffset = io.lcdc.bgWindowDataArea
        ? 0x0000
        : 0x0800;

    const auto windowTileMapOffset = io.lcdc.windowTileMapArea
        ? 0x1c00
        : 0x1800;

    const auto scx = io.scx;
    const auto scy = io.scy;

    const uint8_t* tileData = vram.data + bgWindowDataOffset;

    for (int mapY = 0; mapY < GB_TILE_DATA_RESY / GB_TILE_RESY; ++mapY)
    {
        for (int mapX = 0; mapX < GB_TILE_DATA_RESX / GB_TILE_RESX; ++mapX)
        {
            const auto tileIndex = vram.data[windowTileMapOffset + mapY * 32 + mapX];
            const auto tile = tileData + tileIndex * GB_TILE_BYTES;

            for (int j = 0; j < GB_TILE_RESY; ++j)
            {
                const auto byte1 = tile[j * 2];
                const auto byte2 = tile[j * 2 + 1];

                const auto y = (mapY * GB_TILE_RESY + j + GB_TILE_DATA_RESY - scy) % GB_TILE_DATA_RESY;

                if (y >= GB_LCD_RESY)
                {
                    continue;
                }

                for (int i = 0; i < GB_TILE_RESX; ++i)
                {
                    const auto color
                        = (((byte1 >> (7 - i)) & 1))
                        | (((byte2 >> (7 - i)) & 1) << 1);

                    const auto x = (mapX * GB_TILE_RESX + i + GB_TILE_DATA_RESX - scx) % GB_TILE_DATA_RESX;

                    if (x >= GB_LCD_RESX)
                    {
                        break;
                    }

                    ImageDrawPixel(&screenImage, x, y, colors[color]);
                }
            }
        }
    }

    if (io.lcdc.objEnable)
    {
        uint8_t objHeight = io.lcdc.objSize
            ? 16
            : 8;

        uint8_t objSize = (objHeight * 8) / 4;

        constexpr uint16_t objTileData = 0x0000;

        (void)(objHeight and objSize and objTileData);

        for (uint16_t i = 0; i < 0xa0; i += 4)
        {
            auto y = oam.data[i];
            auto x = oam.data[i + 1];
            auto tileId = oam.data[i + 2];
            auto attr = oam.data[i + 3];

            (void)(attr and x and y and tileId);
        }
    }

    UpdateTexture(screenTexture, screenImage.data);

    RENDER()
    {
        ClearBackground(WHITE);
        DrawTextureEx(screenTexture, Vector2{0, 0}, 0.0f, SCALING, WHITE);
        DrawFPS(SCALING * GB_LCD_RESX - 100, 20);
    }
}

void Video::IO::store(uint8_t addr, uint8_t value)
{
    if (addr == offsetof(IOImpl, dma))
    {
        gb.events.scheduleEvent(dma, gb.cpu.cycles + 620);
    }
    return BaseIO::store(addr, value);
}

uint8_t Video::IO::load(uint8_t addr) const
{
    return BaseIO::load(addr);
}

}  // namespace ppu
