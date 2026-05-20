#pragma once

#include <cstdint>

#include "utils/inline.hpp"

namespace cpu
{

struct Exception
{
    enum Type
    {
        None,
        NotImplemented,
        InvalidOpcode,
        SegmentationFault,
        InfiniteLoop,
        Halt,
        UserInterruption,
    };

    ALWAYS_INLINE constexpr operator bool() const
    {
        return type != None;
    }

    ALWAYS_INLINE constexpr Exception& operator=(Type t)
    {
        type = t;
        return *this;
    }

    ALWAYS_INLINE constexpr void clear()
    {
        type = None;
    }

    ALWAYS_INLINE void reportUserInterruption()
    {
        type = UserInterruption;
    }

    ALWAYS_INLINE void reportSegmentationFault(uint16_t at, bool write)
    {
        type = SegmentationFault;
        segmentationFault.addr = at;
        segmentationFault.write = write;
    }

    ALWAYS_INLINE void reportNotImplemented(uint8_t opcode)
    {
        value = opcode;
        type = NotImplemented;
    }

    ALWAYS_INLINE void reportInvalidOpcode(uint16_t opcode)
    {
        value = opcode;
        type = InvalidOpcode;
    }

    ALWAYS_INLINE void reportInfiniteLoop()
    {
        type = InfiniteLoop;
    }

    ALWAYS_INLINE void reportHalt()
    {
        type = Halt;
    }

    Type     type = Type::None;
    union
    {
        struct
        {
            uint16_t addr;
            bool     write;
        } segmentationFault;
        struct
        {
            uint16_t value;
        } opcode;
        uint16_t value;
    };
};

}  // namespace cpu
