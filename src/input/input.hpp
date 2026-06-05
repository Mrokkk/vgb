#pragma once

#include <cstdint>

#include "fwd.hpp"

namespace input
{

struct Input
{
    Input();
    ~Input();

    void start(const Config& config);
    void stop();
    void reset();

    void update();
    void store(uint8_t value);
    uint8_t load();

private:
    uint8_t mValue;
};

}  // namespace input
