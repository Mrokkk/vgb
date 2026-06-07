#include "timer.hpp"

#include <cstring>

#include "event.hpp"
#include "game_boy.hpp"

namespace cpu
{

static Event divEvent = Event::repeating({
    .name = "DIV",
    .prio   = 0,
    .period = 256,
});

static Event timaEvent = Event::repeating({
    .name = "TIMA",
    .prio   = 0,
    .period = 0,
});

void Timer::start()
{
    divEvent.setCallback(
        [this](size_t)
        {
            div++;
        });

    timaEvent.setCallback(
        [this](size_t)
        {
            if (++tima == 0)
            {
                tima = tma;
                gb.cpu.raiseIrq(cpu::IRQ::Timer);
            }
        });

    gb.events.scheduleEvent(divEvent, 256);
}

void Timer::reset()
{
    memset(values, 0, sizeof(values));
    start();
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
        case 0: duration = 256 * 4; break;
        case 1: duration = 4 * 4; break;
        case 2: duration = 16 * 4; break;
        case 3: duration = 64 * 4; break;
        default: duration = 256;
    }

    gb.events.cancelEvent(timaEvent);
    timaEvent.setPeriod(duration);
    gb.events.scheduleEvent(timaEvent, cycles + duration);
}

}  // namespace cpu
