#pragma once

#include <cstdint>

namespace cpu::isa
{

struct Operand
{
    enum Type : uint8_t
    {
        None,

        Builtin,

        // Immediate
        ImmS8,
        ImmU8,
        Imm16,

        // Address
        Addr8,
        Addr16,

        // 8-bit registers
        A,
        B,
        C,
        D,
        E,
        H,
        L,

        // 16-bit registers
        AF,
        BC,
        DE,
        HL,
        SP,

        SP_Plus_ImmS8,

        // Flags
        FlagZ,
        FlagNZ,
        FlagC,
        FlagNC,
    };

    enum class Action : uint8_t
    {
        None,
        Increment,
        Decrement,
    };

    Type    type:5     = None;
    Action  action:2   = Action::None;
    bool    indirect:1 = false;
    uint8_t value      = 0;
};

}  // namespace cpu::isa
