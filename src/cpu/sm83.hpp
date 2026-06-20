#pragma once

#include <cstddef>
#include <cstdint>

#include "cpu/exception.hpp"
#include "cpu/isa/instruction_set.hpp"
#include "cpu/register.hpp"
#include "memory/memory.hpp"
#include "utils/immobile.hpp"
#include "utils/inline.hpp"

namespace cpu
{

union FlagsRegister
{
    struct
    {
        uint8_t reserved:4;
        uint8_t c:1;
        uint8_t h:1;
        uint8_t n:1;
        uint8_t z:1;
    };
    Register8   value;
};

#define DEFINE_REGISTERS_PAIR(REG1, REG2) \
    union \
    { \
        struct \
        { \
            Register8 REG2; \
            Register8 REG1; \
        }; \
        Register16    REG1 ## REG2; \
    }

#define DEFINE_AF_REGISTERS() \
    union \
    { \
        struct \
        { \
            FlagsRegister f; \
            Register8     a; \
        }; \
        Register16        af; \
    }

#define SM83_REGISTERS \
    struct \
    { \
        DEFINE_AF_REGISTERS(); \
        DEFINE_REGISTERS_PAIR(b, c); \
        DEFINE_REGISTERS_PAIR(d, e); \
        DEFINE_REGISTERS_PAIR(h, l); \
        Register16 sp; \
        Register16 pc; \
        Register8  ie; \
        Register8  ime; \
        Register8  $if; \
    }

struct Regs final
{
    SM83_REGISTERS;
};

enum class IRQ : uint8_t
{
    VBlank = 0,
    LCD    = 1,
    Timer  = 2,
    Serial = 3,
    Joypad = 4,
};

ALWAYS_INLINE static bool checkIrq(IRQ irq, uint8_t reg)
{
    return reg & 1 << uint8_t(irq);
}

struct SM83 final : utils::Immobile
{
    enum class State : uint8_t
    {
        Running,
        Halted,
    };

    SM83();
    ~SM83();

    void reset();
    int step();
    int execute(const isa::Opcode& opcode, isa::InstructionData data, bool prefixed);
    void stop();

    ALWAYS_INLINE void raiseIrq(IRQ irq)
    {
        $if |= 1 << uint8_t(irq);
        state = State::Running;
    }

    ALWAYS_INLINE void clearIrq(IRQ irq)
    {
        $if &= ~(1 << uint8_t(irq));
    }

    void scheduleEi();

private:
    void clear();
    bool isIrqActive(IRQ irq) const;
    void handleIrq(IRQ irq);

public:
    union
    {
        SM83_REGISTERS;
        Regs regs;
    };

    Regs                 lastRegs;
    isa::InstructionData cache;
    Exception            exc;
    State                state;
    size_t               cycles;
    size_t               instructions;
    memory::Memory       mem;
    isa::InstructionSet  isa;
};

}  // namespace cpu
