#include "sm83.hpp"

#include <fmt/base.h>

#include "cpu/isa/opcode.hpp"
#include "cpu/isa/printers.hpp"
#include "cpu/printers.hpp"
#include "cpu/register.hpp"
#include "game_boy.hpp"
#include "utils/colors.hpp"

namespace cpu
{

enum : uint8_t
{
    PREFIX     = 0xcb,
    IRQ_CYCLES = 20,
};

SM83::SM83()
{
    reset();
}

SM83::~SM83() = default;

void SM83::run()
{
    timer.start();
    while (step() == 0 and not stopped);
}

void SM83::reset()
{
    af  = 0;
    bc  = 0;
    de  = 0;
    hl  = 0;
    pc  = 0;
    sp  = 0;
    ie  = 0;
    ime = 0;

    stopped = false;

    halt         = false;
    cycles       = 0;
    instructions = 0;
    exc          = Exception::None;
}

int SM83::step()
{
    cache.clear();

    if (ime and $if)
    {
#define HANDLE_IRQ(IRQ) \
    if (ie & (1 << uint8_t(IRQ))) { handleIrq((uint8_t)IRQ); goto irqHandled; }

        HANDLE_IRQ(IRQ::VBlank);
        HANDLE_IRQ(IRQ::LCD);
        HANDLE_IRQ(IRQ::Timer);
        HANDLE_IRQ(IRQ::Serial);
        HANDLE_IRQ(IRQ::Joypad);
    }

    if (halt)
    {
        if (ime == 0) [[unlikely]]
        {
            fmt::println("CPU is halted with IME == 0");
            return 1;
        }

        cycles = gb.events.performNextEvent();
        return 0;
    }

irqHandled:
    bool prefixed = false;

    uint8_t pcValue = cache.appendOpcodeByte(mem.load8(pc++));

    if (pcValue == PREFIX)
    {
        prefixed = true;
        pcValue = cache.appendOpcodeByte(mem.load8(pc++));
    }

    const auto& opcode = isa.getOpcode(prefixed, pcValue);

    for (uint8_t i = cache.bytes; i < opcode.bytes; ++i)
    {
        cache.appendImmByte(mem.load8(pc++));
    }

    if (execute(opcode, cache, prefixed)) [[unlikely]]
    {
        handleException();
        return 1;
    }

    gb.events.update(cycles);

    return 0;
}

void SM83::stop()
{
    stopped = true;
}

int SM83::execute(const cpu::isa::Opcode& opcode, cpu::isa::InstructionData data, bool prefixed)
{
    auto instruction = isa.getInstruction(prefixed, opcode.value);

    if (not instruction) [[unlikely]]
    {
        exc.reportNotImplemented(opcode.value);
        return 1;
    }

    isa::Immediate immediate{data.imm()};

    instruction(immediate, *this);

    if (exc) [[unlikely]]
    {
        regs = lastRegs;
        return 1;
    }

    instructions++;
    cycles += opcode.cycles;

    lastRegs = regs;

    return 0;
}

void SM83::handleIrq(uint8_t irq)
{
    $if &= ~(1 << irq);
    ime = 0;
    sp -= 2;
    mem.store16(sp, pc - halt);
    pc = 0x40 + 0x08 * irq;
    cycles += IRQ_CYCLES;
    halt = false;
}

void SM83::handleException()
{
    fmt::print("\n" COLOR_RED "Exception raised" COLOR_RESET ": {}\n\n", exc);
    fmt::print("T-cycles: {}\n", cycles);
    fmt::print("Instructions: {}\n", instructions);
}

}  // namespace cpu
