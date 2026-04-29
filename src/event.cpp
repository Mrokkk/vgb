#include "event.hpp"

#if 0
#include <algorithm>
#include <cstdint>
#include <cstring>

#include "game_boy.hpp"

Events::Events()
    : mEventCount(0)
{
    for (int i = 0; i < MAX_EVENT_COUNT; ++i)
    {
        mEvents[i].when = SIZE_MAX;
    }
}

Events::~Events() = default;

void Events::scheduleEvent(size_t when, Event::Callback callback)
{
    if (mEventCount == MAX_EVENT_COUNT) [[unlikely]]
    {
        gb.cpu.exc.reportNoFreeEvent();
        return;
    }
    auto& event = mEvents[mEventCount++];
    event.when = when;
    event.callback = std::move(callback);
    if (mEventCount > 1)
    {
        std::sort(
            mEvents, mEvents + mEventCount,
            [](const auto& lhs, const auto& rhs)
            {
                return lhs.when < rhs.when;
            });
    }
}

void Events::update(size_t cycles)
{
    Event::Callback callbacks[MAX_EVENT_COUNT];
    uint8_t count = 0;
    Event* it = mEvents;

    while (it->when <= cycles)
    {
        callbacks[count++] = std::move(it->callback);
        it->when = SIZE_MAX;
        ++it;
    }

    if (count == 0)
    {
        return;
    }

    mEventCount -= count;

    for (uint8_t i = mEventCount; i >= count; --i)
    {
        mEvents[i - count] = std::move(mEvents[i]);
    }

    for (uint8_t i = 0; i < count; ++i)
    {
        callbacks[i](cycles);
    }
}
#endif
