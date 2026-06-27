#include "sm83.hpp"

#include <cstring>
#include <fmt/base.h>

#include "cpu/exception.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/register.hpp"
#include "event.hpp"
#include "event_system.hpp"
#include "game_boy.hpp"
#include "serializator.hpp"

namespace cpu
{

enum : uint8_t
{
    PREFIX     = 0xcb,
    IRQ_CYCLES = 20,
};

static Event ei = Event::oneShot({
    .name = "EI",
    .prio = 0,
    .callback = [](size_t){ gb.cpu.ime = 1; }
});

SM83::SM83()
{
    clear();
    Serializator::registerData(regs);
    Serializator::registerData(lastRegs);
    Serializator::registerData(reinterpret_cast<void*>(&exc), sizeof(exc));
    Serializator::registerData(state);
    Serializator::registerData(cycles);
    Serializator::registerData(instructions);
    Serializator::registerData(reinterpret_cast<void*>(&mem), sizeof(mem));
    Serializator::registerData(callstack);
    Serializator::registerEvents({&ei});
}

SM83::~SM83() = default;

void SM83::reset()
{
    clear();
    mem.reset();
}

int SM83::step()
{
    cache.clear();

    if (ime and $if)
    {
#define HANDLE_IRQ(IRQ) \
    if (isIrqActive(IRQ)) \
    { \
        handleIrq(IRQ); \
        return 0; \
    }

        HANDLE_IRQ(IRQ::VBlank);
        HANDLE_IRQ(IRQ::LCD);
        HANDLE_IRQ(IRQ::Timer);
        HANDLE_IRQ(IRQ::Serial);
        HANDLE_IRQ(IRQ::Joypad);
    }

    if (state == State::Halted)
    {
        cycles = gb.events.performNextEvent();
        return 0;
    }

    bool prefixed = false;

    const uint16_t oldPc = pc;

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
        if (exc.type != Exception::UserInterruption)
        {
            pc = oldPc;
        }
        return 1;
    }

    gb.events.update(cycles);

    return 0;
}

void SM83::stop()
{
}

void SM83::scheduleEi()
{
    gb.events.cancelEvent(ei);
    gb.events.scheduleEvent(ei, cycles + 4);
}

int SM83::execute(const cpu::isa::Opcode& opcode, cpu::isa::InstructionData data, bool prefixed)
{
    const auto instruction = isa.getInstruction(prefixed, opcode.value);

    if (not instruction) [[unlikely]]
    {
        exc.reportNotImplemented(opcode.value);
        return 1;
    }

    const isa::Immediate immediate{data.imm()};

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

void SM83::clear()
{
    memset(&regs, 0, sizeof(regs));
    memset(&callstack, 0, sizeof(callstack));

    state        = State::Running;
    cycles       = 0;
    instructions = 0;
    exc          = Exception::None;
}

bool SM83::isIrqActive(IRQ irq) const
{
    auto val = 1 << uint8_t(irq);
    return ($if & val) and ($if & val) == (ie & val);
}

void SM83::handleIrq(IRQ irq)
{
    clearIrq(irq);
    ime = 0;
    sp -= 2;
    mem.store16(sp, pc);
    pushStackFrame();
    pc = 0x40 + 0x08 * uint8_t(irq);
    cycles += IRQ_CYCLES;
    state = State::Running;
}

void SM83::pushStackFrame()
{
    callstack.push(pc, gb.cartridge.getRomBank());
}

void SM83::popStackFrame()
{
    callstack.pop();
}

}  // namespace cpu
