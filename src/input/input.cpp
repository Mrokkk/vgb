#include "input.hpp"

#include <fmt/base.h>
#include <raylib.h>

#include "cpu/sm83.hpp"
#include "event.hpp"
#include "event_system.hpp"
#include "game_boy.hpp"

namespace input
{

static Event refresh = Event::repeating({
    .name = "Input",
    .prio = 0,
    .period = 70224,
});

Input::Input()
    : mValue(0xff)
{
}

Input::~Input() = default;

union Directional
{
    struct
    {
        bool right:1;
        bool left:1;
        bool up:1;
        bool down:1;
    };
    uint8_t value;
};

union Buttons
{
    struct
    {
        bool a:1;
        bool b:1;
        bool select:1;
        bool start:1;
    };
    uint8_t value;
};

static Buttons buttons;
static Directional directional;

void Input::start(const Config&)
{
    refresh.setCallback(
        [this](size_t)
        {
            update();
        });

    gb.events.scheduleEvent(refresh, 70224);
}

void Input::reset()
{
    buttons.value = 0;
    directional.value = 0;
    mValue = 0xff;
    gb.events.scheduleEvent(refresh, 70224);
}

#define CHECK_KEY(KEY, BUTTON) \
    do \
    { \
        const auto down = IsKeyDown(KEY); \
        if (down and not (BUTTON)) \
        { \
            BUTTON = true; \
            raiseIrq = true; \
        } \
        else if (not down and (BUTTON)) \
        { \
            BUTTON = false; \
            raiseIrq = false; \
        } \
    } \
    while (0)

void Input::update()
{
    bool raiseIrq = false;
    CHECK_KEY(KEY_X, buttons.a);
    CHECK_KEY(KEY_Z, buttons.b);
    CHECK_KEY(KEY_ENTER, buttons.start);
    CHECK_KEY(KEY_BACKSPACE, buttons.select);

    CHECK_KEY(KEY_LEFT, directional.left);
    CHECK_KEY(KEY_RIGHT, directional.right);
    CHECK_KEY(KEY_UP, directional.up);
    CHECK_KEY(KEY_DOWN, directional.down);

    if (raiseIrq)
    {
        gb.cpu.raiseIrq(cpu::IRQ::Joypad);
    }
}

void Input::store(uint8_t value)
{
    mValue = value | 0xf;
}

uint8_t Input::load()
{
    if ((mValue & (1 << 5)) == 0)
    {
        return (mValue & 0xf0) | ~buttons.value;
    }
    else if ((mValue & (1 << 4)) == 0)
    {
        return (mValue & 0xf0) | ~directional.value;
    }
    return mValue;
}

}  // namespace input
