#include "ppu.hpp"

#include <cstdio>
#include <cstring>

#include <fmt/base.h>

#include "component.hpp"
#include "config.hpp"
#include "cpu/sm83.hpp"
#include "event.hpp"
#include "fwd.hpp"
#include "game_boy.hpp"
#include "memory/generic.hpp"
#include "serializator.hpp"
#include "utils/inline.hpp"
#include "utils/unique_ptr.hpp"

struct Ppu final : Component
{
    Ppu(const Config& config);

    void reset() override;

    void store(uint16_t address, uint8_t value) override;
    uint8_t load(uint16_t address) const override;

    void initPpu();
    void drawLine();

    ALWAYS_INLINE void mode0Callback();
    ALWAYS_INLINE void mode1Callback();
    ALWAYS_INLINE void mode2Callback();
    ALWAYS_INLINE void mode3Callback();
    ALWAYS_INLINE void dmaCallback();

    ALWAYS_INLINE void updateBgPalette();
    ALWAYS_INLINE void updateObjPalette();

    struct IOImpl;
    IOImpl& getIo();

    using BaseIO = memory::GenericIO<0xff4c - 0xff40>;

    struct IO : BaseIO
    {
        void store(uint8_t addr, uint8_t value);
        uint8_t load(uint8_t addr) const;
    };

    bool      graphical;
    Event     dma;
    Event     mode0;
    Event     mode1;
    Event     mode2;
    Event     mode3;
    IO        io;
    uint8_t   bgPalette[4];
    uint8_t   objPalette[8];
};

enum
{
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

struct Ppu::IOImpl
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

Ppu::Ppu(const Config& config)
    : graphical(config.videoConfig == VideoConfig::Graphical)
    , dma(Event::oneShot({
        .name = "OAM DMA",
        .prio = 0,
        .callback = [this](size_t){ dmaCallback(); }
    }))
    , mode0(Event::repeating({
        .name = "PPU Mode 0",
        .prio = 0,
        .period = HSYNC_DURATION,
        .callback = [this](size_t){ mode0Callback(); }
    }))
    , mode1(Event::repeating({
        .name = "PPU Mode 1",
        .prio = 0,
        .period = FRAME_DURATION,
        .callback = [this](size_t){ mode1Callback(); }
    }))
    , mode2(Event::repeating({
        .name = "PPU Mode 2",
        .prio = 1,
        .period = HSYNC_DURATION,
        .callback = [this](size_t){ mode2Callback(); }
    }))
    , mode3(Event::repeating({
        .name = "PPU Mode 3",
        .prio = 0,
        .period = HSYNC_DURATION,
        .callback = [this](size_t){ mode3Callback(); }
    }))
{
    Serializator::registerData(io.data);
    Serializator::registerData(bgPalette);
    Serializator::registerData(objPalette);
    Serializator::registerEvents({&dma, &mode0, &mode1, &mode2, &mode3});
    initPpu();
}

ALWAYS_INLINE Ppu::IOImpl& Ppu::getIo()
{
    return *reinterpret_cast<Ppu::IOImpl*>(io.data);
}

void Ppu::reset()
{
    initPpu();
}

void Ppu::store(uint16_t address, uint8_t value)
{
    if (address == offsetof(IOImpl, dma))
    {
        gb.events.scheduleEvent(dma, gb.cpu.cycles + DMA_DURATION);
    }
    else if (address == offsetof(IOImpl, bgp))
    {
        auto& io = getIo();
        io.bgp = value;
        updateBgPalette();
        return;
    }
    else if (address == offsetof(IOImpl, obp0))
    {
        auto& io = getIo();
        io.obp0 = value;
        updateObjPalette();
        return;
    }
    else if (address == offsetof(IOImpl, obp1))
    {
        auto& io = getIo();
        io.obp1 = value;
        updateObjPalette();
        return;
    }
    return io.store(address, value);
}

uint8_t Ppu::load(uint16_t address) const
{
    return io.load(address);
}

void Ppu::initPpu()
{
    auto& ioRo = *reinterpret_cast<IOImpl*>(io.roMasks);

    ioRo.stat.ppuMode = 3;
    ioRo.stat.lycEqLy = 1;
    ioRo.stat.reserved = 1;
    ioRo.ly = 0xff;

    auto& io = getIo();
    io.ly = VSYNC_LY_END - 1;

    gb.events.scheduleEvent(mode0, 369);
    gb.events.scheduleEvent(mode1, FRAME_DURATION - 4560);
    gb.events.scheduleEvent(mode2, 0);
    gb.events.scheduleEvent(mode3, 80);

    updateBgPalette();
    updateObjPalette();
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

void Ppu::drawLine()
{
    auto& io = getIo();

    const uint8_t y = io.ly;
    const uint8_t wx = io.wx - 7;

    const bool useWindow = io.lcdc.windowEnable and io.wy <= io.ly;
    bool unsig;

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

    auto& oam = gb.cpu.mem.oam;
    auto& vram = gb.cpu.mem.vram;

    if (io.lcdc.objEnable)
    {
        for (uint16_t i = 0; i < GB_OAM_SIZE and objIndex < 10; i += sizeof(Object))
        {
            const auto obj = reinterpret_cast<const Object*>(oam.data + i);

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

        bool gotObj = false;
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
                    gb.renderer->drawPixel(x, y, objPalette[color + obj->attr.dmgPalette * 4]);
                    gotObj = true;
                }
                break;
            }
        }
        if (not gotObj)
        {
            gb.renderer->drawPixel(x, y, bgPalette[bgColor]);
        }
    }
}

ALWAYS_INLINE void Ppu::mode0Callback()
{
    auto& io = getIo();
    if (io.ly < 144)
    {
        drawLine();
        io.stat.ppuMode = 0;
        if (io.stat.intMode0)
        {
            gb.cpu.raiseIrq(cpu::IRQ::LCD);
        }
    }
}

ALWAYS_INLINE void Ppu::mode1Callback()
{
    gb.frame();
    auto& io = getIo();
    io.stat.ppuMode = 1;
    gb.cpu.raiseIrq(cpu::IRQ::VBlank);
    if (io.stat.intMode1)
    {
        gb.cpu.raiseIrq(cpu::IRQ::LCD);
    }
}

ALWAYS_INLINE void Ppu::mode2Callback()
{
    auto& io = getIo();

    if (++io.ly < 144)
    {
        io.stat.ppuMode = 2;
        if (io.stat.intMode2)
        {
            gb.cpu.raiseIrq(cpu::IRQ::LCD);
        }
    }
    if (io.stat.intLyc and io.ly == io.lyc)
    {
        gb.cpu.raiseIrq(cpu::IRQ::LCD);
    }
}

ALWAYS_INLINE void Ppu::mode3Callback()
{
    auto& io = getIo();
    if (io.ly < 144)
    {
        io.stat.ppuMode = 3;
    }
}

void Ppu::dmaCallback()
{
    auto& io = getIo();
    auto& mem = gb.cpu.mem;
    uint16_t src = ((uint16_t)io.dma << 8);

    for (uint16_t i = 0; i < GB_OAM_SIZE; ++i)
    {
        mem.oam.data[i] = mem.load8(src + i);
    }
}

ALWAYS_INLINE void Ppu::updateBgPalette()
{
    auto& io = getIo();
    for (int i = 0; i < 4; ++i)
    {
        bgPalette[i] = getColorFromPalette(io.bgp, i);
    }
}

ALWAYS_INLINE void Ppu::updateObjPalette()
{
    auto& io = getIo();
    for (int i = 0; i < 8; ++i)
    {
        objPalette[i] = getColorFromPalette(*(&io.obp0 + i / 4) & ~3, i % 4);
    }
}

void Ppu::IO::store(uint8_t addr, uint8_t value)
{
    return BaseIO::store(addr, value);
}

uint8_t Ppu::IO::load(uint8_t addr) const
{
    return BaseIO::load(addr);
}

void createPpu(GameBoy& gb, const Config& config)
{
    gb.registerComponent(Component::Ppu, utils::makeUnique<Ppu>(config));
}
