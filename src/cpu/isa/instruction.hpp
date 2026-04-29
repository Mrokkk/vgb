#pragma once

#include <cstdint> // IWYU pragma: keep

#include "cpu/fwd.hpp"
#include "utils/inline.hpp"

namespace cpu::isa
{

struct Immediate
{
    ALWAYS_INLINE uint8_t load8() const
    {
        return value;
    }

    ALWAYS_INLINE uint16_t load16() const
    {
        return value;
    }

    uint16_t value;
};

using Instruction = void (*)(Immediate immediate, SM83& cpu);

#define INSTRUCTION_NAME(NAME, OPCODE) \
    NAME ## _ ## OPCODE

#define DECLARE_INSTRUCTION(NAME, OPCODE) \
    void INSTRUCTION_NAME(NAME, OPCODE)(Immediate imm, SM83& cpu)

#define DEFINE_INSTRUCTION(NAME, OPCODE, ...) \
    void INSTRUCTION_NAME(NAME, OPCODE)( \
        [[maybe_unused]] Immediate imm, \
        [[maybe_unused]] SM83& cpu) \
    { \
        __VA_ARGS__; \
    }

}  // namespace cpu::isa
