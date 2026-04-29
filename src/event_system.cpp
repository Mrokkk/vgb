#include "event_system.hpp"

#include "event.hpp"

EventSystem::EventSystem()
    : mHead(new Event{})
{
    mHead->next = mHead;
    mHead->prev = mHead;
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

        if (cur->type == Event::Repeating)
        {
            cur->when = cycles + cur->period;
            scheduleEvent(*cur);
        }

        cur->callback(cycles);
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

    if (event->type == Event::Repeating)
    {
        event->when = cycles + event->period;
        scheduleEvent(*event);
    }

    event->callback(cycles);

    return cycles;
}

void EventSystem::scheduleEvent(Event& event)
{
    Event* cur = mHead->next;

    while (cur != mHead)
    {
        if (event.when < cur->when or (event.when == cur->when and event.prio > cur->prio))
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
}
