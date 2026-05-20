#pragma once

#include <cstddef>

struct Event;

struct EventSystem
{
    EventSystem();
    ~EventSystem();

    void update(size_t cycles);
    size_t performNextEvent();
    void scheduleEvent(Event& event, size_t when);
    void cancelEvent(Event& event);
    void reset();

private:
    Event* mHead;
};
