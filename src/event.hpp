#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

struct Event
{
    using Callback = std::move_only_function<void(size_t)>;

    enum Type : uint8_t
    {
        Repeating,
        OneShot,
    };

    uint8_t  prio;
    Type     type;
    size_t   when;
    size_t   period;
    Callback callback;
    Event*   prev;
    Event*   next;
};
