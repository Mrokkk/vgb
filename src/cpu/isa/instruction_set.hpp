#pragma once

#include "cpu/isa/instruction.hpp"
#include "cpu/isa/opcode.hpp"
#include "utils/inline.hpp"

namespace cpu::isa
{

struct InstructionSet
{
    InstructionSet();

    ALWAYS_INLINE const Opcode& getOpcode(bool prefixed, uint8_t opcode) const
    {
        return mOpcodes[opcode + 256 * static_cast<int>(prefixed)];
    }

    ALWAYS_INLINE Instruction getInstruction(bool prefixed, uint8_t opcode) const
    {
        return mInstructions[opcode + 256 * static_cast<int>(prefixed)];
    }

    ALWAYS_INLINE const Opcode* getOpcodes() const
    {
        return mOpcodes;
    }

private:
    Opcode      mOpcodes[512];
    Instruction mInstructions[512];
};

}  // namespace cpu::isa
