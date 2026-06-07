#include "event_system.hpp"

#include <cassert>
#include <cstdlib>

#include "event.hpp"
#include "fmt/base.h"

EventSystem::EventSystem()
    : mHead(new Event())
{
}

EventSystem::~EventSystem()
{
    delete mHead;
}

void EventSystem::update(size_t cycles)
{
    while (mHead->next != mHead)
    {
        auto cur = mHead->next;

        if (cycles < cur->when)
        {
            break;
        }

        cancelEvent(*cur);

        if (cur->type == Event::Type::Repeating)
        {
            scheduleEvent(*cur, cycles + cur->data.period);
        }

        cur->data.callback(cycles);
    }
}

size_t EventSystem::performNextEvent()
{
    auto event = mHead->next;

    if (event == mHead) [[unlikely]]
    {
        return 0;
    }

    auto cycles = event->when;

    cancelEvent(*event);

    if (event->type == Event::Type::Repeating)
    {
        scheduleEvent(*event, cycles + event->data.period);
    }

    event->data.callback(cycles);

    return cycles;
}

void EventSystem::scheduleEvent(Event& event, size_t when)
{
    assert(event.data.callback);
    assert(event.next == &event);
    assert(event.prev == &event);

    event.when = when;

    Event* cur = mHead->next;

    int iteration = 0;

    while (cur != mHead)
    {
        if (iteration == 128) [[unlikely]]
        {
            fmt::println("Bug in EventSystem; infinite loop detected");
            std::abort();
        }
        if (event.when < cur->when or (event.when == cur->when and event.data.prio > cur->data.prio))
        {
            auto prev = cur->prev;
            auto next = cur;

            next->prev = &event;
            prev->next = &event;
            event.next = next;
            event.prev = prev;

            return;
        }

        cur = cur->next;
        iteration++;
    }

    auto last = mHead->prev;
    last->next = &event;
    mHead->prev = &event;
    event.prev = last;
    event.next = mHead;
}

void EventSystem::cancelEvent(Event& event)
{
    auto prev = event.prev;
    auto next = event.next;
    prev->next = next;
    next->prev = prev;
    event.prev = &event;
    event.next = &event;
}

void EventSystem::reset()
{
    while (mHead->next != mHead)
    {
        cancelEvent(*mHead->next);
    }
}

void EventSystem::forEachEvent(utils::FunctionRef<void(const Event&)> callback) const
{
    auto event = mHead->next;
    while (event != mHead)
    {
        callback(*event);
        event = event->next;
    }
}
