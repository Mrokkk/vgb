#include "timer.hpp"

#include "event.hpp"
#include "game_boy.hpp"

namespace cpu
{

Event divEvent{
    .prio = 0,
    .type = Event::Repeating,
    .when = 256,
    .period = 256,
};

void Timer::start()
{
    divEvent.callback =
        [this](size_t)
        {
            div++;
        };
    gb.events.scheduleEvent(divEvent);
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
                //scheduleTima(gb.cpu.cycles);
            }
            return;
    }
    gb.cpu.exc.reportSegmentationFault(addr, true);
}

uint8_t Timer::load(uint16_t addr) const
{
    return values[addr];
}

void Timer::scheduleDiv(size_t cycles)
{
    (void)cycles;
    //gb.events.scheduleEvent(
        //cycles + 256,
        //[this](size_t cycles)
        //{
            //div++;
            //scheduleDiv(cycles);
            //return 0;
        //});
}

void Timer::scheduleTima(size_t cycles)
{
    size_t duration = 256;
    switch (tac.clockSelect)
    {
        case 0: duration = 256 * 4; break;
        case 1: duration = 4 * 4; break;
        case 2: duration = 16 * 4; break;
        case 3: duration = 64 * 4; break;
    }
    (void)(duration and cycles);
    //gb.events.scheduleEvent(
        //cycles + duration,
        //[this](size_t cycles)
        //{
            //if (++tima == 0)
            //{
                //tima = tma;
                //gb.cpu.$if |= 1 << 2;
            //}
            //scheduleTima(cycles);
            //return 0;
        //});
}

}  // namespace cpu
