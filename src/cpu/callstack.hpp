#pragma once

#include <cstdint>

namespace cpu
{

struct Callstack
{
    struct Frame
    {
        uint8_t romBank;
        uint16_t ret;
    };

    void push(uint16_t pc, uint8_t romBank)
    {
        data[index++] = {
            .romBank = romBank,
            .ret = pc,
        };
    }

    void pop()
    {
        if (index > 0)
        {
            --index;
        }
    }

    uint8_t index;
    Frame   data[128];
};

}  // namespace cpu
