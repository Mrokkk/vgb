#pragma once

#include <cstddef>

struct Event;

struct EventSystem
{
    EventSystem();
    ~EventSystem();

    void update(size_t cycles);
    size_t performNextEvent();
    void scheduleEvent(Event& event);
    void cancelEvent(Event& event);

private:
    Event* mHead;
};
