#pragma once

#include <cstdint>

namespace memory
{

struct IO
{
    void store(uint16_t addr, uint8_t val);
    uint8_t load(uint16_t addr) const;
};

}  // namespace memory
