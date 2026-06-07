#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

struct EventSystem;

struct Event
{
    using Callback = std::move_only_function<void(size_t)>;

    enum class Type : uint8_t
    {
        Repeating,
        OneShot,
    };

    struct Data
    {
        const char* name;
        uint8_t  prio;
        size_t   period;
        Callback callback;
    };

    static Event oneShot(Data data)
    {
        return Event(Type::OneShot, std::move(data));
    }

    static Event repeating(Data data)
    {
        return Event(Type::Repeating, std::move(data));
    }

    void setCallback(Callback cb)
    {
        data.callback = std::move(cb);
    }

    void setPeriod(size_t period)
    {
        data.period = period;
    }

    Type getType() const
    {
        return type;
    }

    size_t getWhen() const
    {
        return when;
    }

    const char* getName() const
    {
        return data.name;
    }

private:
    friend EventSystem;

    Event()
        : prev(this)
        , next(this)
    {
    }

    Event(Type t, Data d)
        : type(t)
        , data(std::move(d))
        , prev(this)
        , next(this)
    {
    }

    Type   type;
    Data   data;
    size_t when;
    Event* prev;
    Event* next;
};
