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

    GB_OAM_SIZE       = 0xa0,

    FRAME_DURATION    = 70224,
    HSYNC_DURATION    = 456,
    DMA_DURATION      = 620,
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

static const Color colors[] = {
    {0xff, 0xff, 0xff, 0xff},
    {0x80, 0x80, 0x80, 0xff},
    {0x50, 0x50, 0x50, 0xff},
    {0x00, 0x00, 0x00, 0xff},
};

Video::Video()
    : mGraphical(false)
{
}

Video::~Video()
{
    if (mGraphical)
    {
        CloseWindow();
    }
}

static Image screenImage;
static Texture2D screenTexture;

static Event hsync = Event::repeating({
    .name = "HSYNC",
    .prio = 0,
    .period = HSYNC_DURATION,
});

static Event dma = Event::oneShot({
    .name = "OAM DMA",
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
    mGraphical = config.videoConfig == VideoConfig::Graphical;

    initPpu();

    if (mGraphical)
    {
        initRenderer();
    }
}

void Video::reset()
{
    memset(io.data, 0, sizeof(io.data));
    memset(oam.data, 0, sizeof(oam.data));
    memset(vram.data, 0, sizeof(vram.data));
    initPpu();
}

void Video::initRenderer()
{
    SetTraceLogCallback(raylibLogFormat);
    InitWindow(GB_LCD_RESX * SCALING, GB_LCD_RESY * SCALING, "GameBoy");

    SetTargetFPS(60);

    screenImage = GenImageColor(GB_LCD_RESX, GB_LCD_RESY, WHITE);
    screenTexture = LoadTextureFromImage(screenImage);
}

void Video::initPpu()
{
    auto& ioRo = *reinterpret_cast<IOImpl*>(io.roMasks);

    ioRo.stat.ppuMode = 3;
    ioRo.stat.lycEqLy = 1;
    ioRo.stat.reserved = 1;
    ioRo.ly = 0xff;

    auto& io = getIo();
    io.ly = VSYNC_LY_START - 1;

    if (mGraphical)
    {
        hsync.setCallback(
            [this](size_t)
            {
                auto& io = getIo();
                if (io.ly < VSYNC_LY_START)
                {
                    drawLine();
                }
                if (++io.ly == VSYNC_LY_START)
                {
                    renderFrame();
                    gb.cpu.raiseIrq(cpu::IRQ::VBlank);
                    io.stat.ppuMode = 1;
                }
                else
                {
                    io.stat.ppuMode = 3;
                }
            });
    }
    else
    {
        hsync.setCallback(
            [this](size_t)
            {
                auto& io = getIo();
                if (++io.ly == VSYNC_LY_START)
                {
                    gb.cpu.raiseIrq(cpu::IRQ::VBlank);
                    io.stat.ppuMode = 1;
                }
                else
                {
                    io.stat.ppuMode = 3;
                }
            });
    }

    dma.setCallback(
        [this](size_t)
        {
            auto& io = getIo();
            uint16_t src = ((uint16_t)io.dma << 8);

            for (uint16_t i = 0; i < GB_OAM_SIZE; ++i)
            {
                oam.data[i] = gb.cpu.mem.load8(src + i);
            }
        });

    gb.events.scheduleEvent(hsync, HSYNC_DURATION);
}

struct Object
{
    uint8_t yPos;
    uint8_t xPos;
    uint8_t tileId;
    union
    {
        struct
        {
            uint8_t cbgPalette:3;
            uint8_t bank:1;
            uint8_t dmgPalette:1;
            uint8_t xflip:1;
            uint8_t yflip:1;
            uint8_t prio:1;
        };
        uint8_t value;
    } attr;
};

static uint8_t getColorFromPalette(uint8_t reg, int index)
{
    return (reg >> (index * 2)) & 3;
}

static uint8_t getColorIndexFromTile(uint8_t* tile, uint8_t pixel)
{
    return (((tile[0] >> (7 - (pixel % 8))) & 1))
        | (((tile[1] >> (7 - (pixel % 8))) & 1) << 1);
}

void Video::drawLine()
{
    auto& io = getIo();

    const uint8_t y = io.ly;
    const uint8_t wx = io.wx - 7;

    const bool useWindow = io.lcdc.windowEnable and io.wy <= io.ly;
    bool unsig;

    Color bgPalette[4];
    Color objPalette[8];

    for (int i = 0; i < 4; ++i)
    {
        bgPalette[i] = colors[getColorFromPalette(io.bgp, i)];
    }

    for (int i = 0; i < 8; ++i)
    {
        objPalette[i] = colors[getColorFromPalette(*(&io.obp0 + i / 4) & ~3, i % 4)];
    }

    uint16_t tileData;

    if (io.lcdc.bgWindowDataArea)
    {
        tileData = 0x0000;
        unsig = true;
    }
    else
    {
        tileData = 0x0800;
        unsig = false;
    }

    const uint16_t bgMemory = useWindow
        ? io.lcdc.windowTileMapArea
            ? 0x1c00
            : 0x1800
        : io.lcdc.bgTileMapArea
            ? 0x1c00
            : 0x1800;

    Object objs[10];
    int objIndex = 0;

    if (io.lcdc.objEnable)
    {
        for (uint16_t i = 0; i < GB_OAM_SIZE and objIndex < 10; i += sizeof(Object))
        {
            const auto obj = reinterpret_cast<Object*>(oam.data + i);

            if ((y + 16 >= obj->yPos) and (y + 8 < obj->yPos))
            {
                objs[objIndex++] = *obj;
            }
        }
    }

    const uint8_t yPos = useWindow
        ? y - io.wy
        : io.scy + y;

    for (uint8_t x = 0; x < GB_LCD_RESX; ++x)
    {
        const uint8_t xPos = useWindow and x >= wx
            ? x - wx
            : io.scx + x;

        const uint16_t tileAddr = bgMemory + (yPos / 8) * 32 + xPos / 8;

        const auto tileId = unsig
            ? vram.data[tileAddr]
            : 128 + (int8_t)vram.data[tileAddr];

        const auto tileLocation = tileData + tileId * GB_TILE_BYTES;
        const auto line = (yPos % 8) * 2;

        const auto bgColor = getColorIndexFromTile(&vram.data[tileLocation + line], xPos % 8);

        ImageDrawPixel(&screenImage, x, y, bgPalette[bgColor]);

        for (int i = 0; i < objIndex; ++i)
        {
            const auto obj = &objs[i];
            if ((x + 8 >= obj->xPos) and (x < obj->xPos))
            { // FIXME: lack of support for 8x16 objects
                const auto xPos = obj->xPos - 8;
                const auto yPos = obj->yPos - 16;
                const auto tileData = vram.data + (obj->attr.bank ? 0x1000 : 0x0);
                const auto tile = tileData + obj->tileId * GB_TILE_BYTES;

                const auto relX = obj->attr.xflip ? 7 - (x - xPos) : x - xPos;
                const auto relY = obj->attr.yflip ? 7 - (y - yPos) : y - yPos;

                const auto color = getColorIndexFromTile(&tile[relY * 2], relX);

                if (color == 0)
                {
                    continue;
                }
                if (not obj->attr.prio or bgColor == 0)
                {
                    ImageDrawPixel(&screenImage, x, y, objPalette[color + obj->attr.dmgPalette * 4]);
                }
                break;
            }
        }
    }
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

    UpdateTexture(screenTexture, screenImage.data);

    RENDER()
    {
        ClearBackground(BLACK);
        DrawTextureEx(screenTexture, Vector2{0, 0}, 0.0f, SCALING, WHITE);
        DrawFPS(SCALING * GB_LCD_RESX - 100, 20);
    }
}

void Video::IO::store(uint8_t addr, uint8_t value)
{
    if (addr == offsetof(IOImpl, dma))
    {
        gb.events.scheduleEvent(dma, gb.cpu.cycles + DMA_DURATION);
    }
    return BaseIO::store(addr, value);
}

uint8_t Video::IO::load(uint8_t addr) const
{
    return BaseIO::load(addr);
}

}  // namespace ppu
