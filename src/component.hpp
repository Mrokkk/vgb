#pragma once

#include <cstdint>

#include "utils/unique_ptr.hpp"

struct Component
{
    enum Type
    {
        Joypad,
        Serial,
        Timer,
        Apu,
        Ppu,
        Last = Ppu,
    };

    virtual ~Component() = default;

    virtual void reset() = 0;
    virtual void store(uint16_t address, uint8_t value) = 0;
    virtual uint8_t load(uint16_t address) const = 0;
};

using ComponentPtr = utils::UniquePtr<Component>;
