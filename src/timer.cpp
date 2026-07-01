#include "timer.hpp"

#include <cstring>

#include "component.hpp"
#include "event.hpp"
#include "game_boy.hpp"
#include "save_serializer.hpp"
#include "utils/unique_ptr.hpp"

struct Timer final : Component
{
    Timer();

    void reset() override;

    void store(uint16_t addr, uint8_t value) override;
    uint8_t load(uint16_t addr) const override;

    void scheduleTima(size_t cycles);

    union
    {
        struct
        {
            uint8_t div;
            uint8_t tima;
            uint8_t tma;

            union
            {
                struct
                {
                    uint8_t clockSelect:2;
                    uint8_t enable:1;
                    uint8_t reserved:5;
                };
                uint8_t value;
            } tac;
        };
        uint8_t values[4];
    };

    Event divEvent;
    Event timaEvent;
};

enum
{
    DIV_PERIOD = 256,
};

Timer::Timer()
    : divEvent(Event::repeating({
        .name = "DIV",
        .prio   = 0,
        .period = DIV_PERIOD,
        .callback = [this](auto){ div++; }
    }))
    , timaEvent(Event::repeating({
        .name = "TIMA",
        .prio   = 0,
        .period = 0,
        .callback =
            [this](auto)
            {
                if (++tima == 0)
                {
                    tima = tma;
                    gb.cpu.raiseIrq(cpu::IRQ::Timer);
                }
            }
    }))
{
    reset();
    SaveSerializer::registerData("timer.values", values);
    SaveSerializer::registerData("timer.divEvent", divEvent);
    SaveSerializer::registerData("timer.timaEvent", timaEvent);
}

void Timer::reset()
{
    memset(values, 0, sizeof(values));
    gb.events.scheduleEvent(divEvent, DIV_PERIOD);
}

void Timer::store(uint16_t addr, uint8_t value)
{
    switch (addr)
    {
        case 0:
            div = 0;
            return;

        case 1:
            tima = value;
            return;

        case 2:
            tma = value;
            return;

        case 3:
            tac.value = value;
            if (tac.enable)
            {
                scheduleTima(gb.cpu.cycles);
            }
            return;
    }
    gb.cpu.exc.reportSegmentationFault(addr, true);
}

uint8_t Timer::load(uint16_t addr) const
{
    return values[addr];
}

void Timer::scheduleTima(size_t cycles)
{
    size_t duration;

    switch (tac.clockSelect)
    {
        case 0: duration  = 256 * 4; break;
        case 1: duration  = 4 * 4; break;
        case 2: duration  = 16 * 4; break;
        case 3: duration  = 64 * 4; break;
        default: duration = 256;
    }

    gb.events.cancelEvent(timaEvent);
    timaEvent.setPeriod(duration);
    gb.events.scheduleEvent(timaEvent, cycles + duration);
}

void createTimer(GameBoy& gb)
{
    gb.registerComponent(Component::Timer, utils::makeUnique<Timer>());
}
