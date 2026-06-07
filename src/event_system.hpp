#pragma once

#include <cstddef>

#include "utils/function_ref.hpp"

struct Event;

struct EventSystem final
{
    EventSystem();
    ~EventSystem();

    void update(size_t cycles);
    size_t performNextEvent();
    void scheduleEvent(Event& event, size_t when);
    void cancelEvent(Event& event);
    void reset();

    void forEachEvent(utils::FunctionRef<void(const Event&)> callback) const;

private:
    Event* mHead;
};
