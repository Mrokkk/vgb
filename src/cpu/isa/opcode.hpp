#pragma once

#include <cstdint>

#include "cpu/isa/operand.hpp"
#include "utils/byte_order.hpp"
#include "utils/inline.hpp"

namespace cpu::isa
{

struct InstructionData
{
    uint8_t bytes = 0;
    uint8_t immStart = 0;
    uint8_t data[4];

    ALWAYS_INLINE uint8_t appendOpcodeByte(uint8_t value)
    {
        ++immStart;
        return data[bytes++] = value;
    }

    ALWAYS_INLINE uint8_t appendImmByte(uint8_t value)
    {
        return data[bytes++] = value;
    }

    ALWAYS_INLINE uint16_t imm() const
    {
        return utils::le16(data[immStart], data[immStart + 1]);
    }

    ALWAYS_INLINE uint8_t immU8() const
    {
        return data[immStart];
    }

    ALWAYS_INLINE int8_t immS8() const
    {
        return (int8_t)data[immStart];
    }

    ALWAYS_INLINE uint16_t immU16() const
    {
        return utils::le16(data[immStart], data[immStart + 1]);
    }

    ALWAYS_INLINE void clear()
    {
        bytes = 0;
        immStart = 0;
    }
};

struct Opcode
{
    enum Type : uint8_t
    {
#define MNEMO(UPPER, LOWER) UPPER,
#define MNEMO_ILL(UPPER, LOWER) UPPER,
#include "cpu/isa/mnemos.hpp"
#undef MNEMO
#undef MNEMO_ILL
    };

    Type    mnemo     = ILL;
    uint8_t opCount   = 0;
    uint8_t value     = 0;
    Operand op[2]     = {
        Operand{.type = Operand::None},
        Operand{.type = Operand::None},
    };
    uint8_t bytes     = 0;
    uint8_t cycles    = 1;
};

const char* opcodeName(Opcode::Type t);

}  // namespace cpu::isa
