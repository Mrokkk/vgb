#include "joypad.hpp"

#include <cstdint>
#include <fmt/base.h>

#include "component.hpp"
#include "cpu/sm83.hpp"
#include "game_boy.hpp"
#include "input.hpp"
#include "save_serializer.hpp"
#include "utils/unique_ptr.hpp"

struct Joypad : Component
{
    Joypad();

    void reset() override;
    void update(GameBoyInput input);

    void store(uint16_t address, uint8_t value) override;
    uint8_t load(uint16_t address) const override;

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

    union JOYP
    {
        Buttons     buttons;
        Directional directional;

        struct
        {
            uint8_t unused:4;
            uint8_t selectDirectional:1;
            uint8_t selectButtons:1;
        };

        struct
        {
            uint8_t lowerNibble:4;
            uint8_t higherNibble:4;
        };

        uint8_t value;
    };

    Buttons     buttons;
    Directional directional;
    JOYP        joyp;
};

Joypad::Joypad()
{
    buttons.value = 0;
    directional.value = 0;
    joyp.value = 0xff;

    gb.input->subscribeForGameBoyInput(
        [this](GameBoyInput input)
        {
            update(input);
        });

    SaveSerializer::registerData(buttons);
    SaveSerializer::registerData(directional);
    SaveSerializer::registerData(joyp);
}

void Joypad::reset()
{
    buttons.value = 0;
    directional.value = 0;
    joyp.value = 0xff;
}

#define CHECK_KEY(KEY, BUTTON) \
    do \
    { \
        const auto down = input.KEY; \
        if (down and not (BUTTON)) \
        { \
            if (0) fmt::println(#KEY " pressed"); \
            BUTTON = true; \
        } \
        else if (not down and (BUTTON)) \
        { \
            if (0) fmt::println(#KEY " released"); \
            BUTTON = false; \
        } \
    } \
    while (0)

void Joypad::update(GameBoyInput input)
{
    auto old = joyp;

    CHECK_KEY(a, buttons.a);
    CHECK_KEY(b, buttons.b);
    CHECK_KEY(start, buttons.start);
    CHECK_KEY(select, buttons.select);

    CHECK_KEY(left, directional.left);
    CHECK_KEY(right, directional.right);
    CHECK_KEY(up, directional.up);
    CHECK_KEY(down, directional.down);

    if (not joyp.selectButtons)
    {
        joyp.lowerNibble = ~buttons.value;
    }
    else if (not joyp.selectDirectional)
    {
        joyp.lowerNibble = ~directional.value;
    }
    else
    {
        joyp.lowerNibble = 0xf;
    }

    if (old.lowerNibble != joyp.lowerNibble)
    {
        gb.cpu.raiseIrq(cpu::IRQ::Joypad);
    }
}

void Joypad::store(uint16_t, uint8_t value)
{
    JOYP old = joyp;

    joyp.higherNibble = value >> 4;

    if (not joyp.selectButtons)
    {
        joyp.lowerNibble = ~buttons.value;
    }
    else if (not joyp.selectDirectional)
    {
        joyp.lowerNibble = ~directional.value;
    }
    else
    {
        joyp.lowerNibble = 0xf;
    }

    if (old.lowerNibble != joyp.lowerNibble)
    {
        gb.cpu.raiseIrq(cpu::IRQ::Joypad);
    }
}

uint8_t Joypad::load(uint16_t) const
{
    return joyp.value;
}

void createJoypad(GameBoy& gb, const Config&)
{
    gb.registerComponent(Component::Joypad, utils::makeUnique<Joypad>());
}
