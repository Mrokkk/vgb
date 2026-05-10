#pragma once

#include <cstddef>
#include <cstdint>

namespace cpu
{

struct Timer
{
    constexpr static size_t inline PERIOD = 256;

    void start();
    void store(uint16_t addr, uint8_t value);
    uint8_t load(uint16_t addr) const;

    union
    {
        struct
        {
            uint8_t div;
            uint8_t tima;
            uint8_t tma;

            union
            {
                struct
                {
                    uint8_t clockSelect:2;
                    uint8_t enable:1;
                    uint8_t reserved:5;
                };
                uint8_t value;
            } tac;
        };
        uint8_t values[4];
    };

private:
    void scheduleTima(size_t cycles);
};

}  // namespace cpu
