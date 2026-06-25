#include "printer.hpp"

#include <fmt/base.h>

#include "cpu/isa/decoder.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/printers.hpp"
#include "cpu/sm83.hpp"
#include "debugger/context.hpp"

namespace debugger
{

void printInstruction(Context& ctx)
{
    bool prefixed = false;

    const auto& cpu = ctx.cpu;
    auto tmpPc = cpu.pc.get();
    cpu::isa::InstructionData tmpCache;

    uint8_t pcValue = tmpCache.appendOpcodeByte(cpu.mem.load8(tmpPc++));

    if (pcValue == 0xcb)
    {
        prefixed = true;
        pcValue = tmpCache.appendOpcodeByte(cpu.mem.load8(tmpPc++));
    }

    const auto& opcode = cpu.isa.getOpcode(prefixed, pcValue);

    for (uint8_t i = tmpCache.bytes; i < opcode.bytes; ++i)
    {
        tmpCache.appendImmByte(cpu.mem.load8(tmpPc++));
    }

    ctx.console.writeLine(
        "{pc:08x}:   {bytes}     | {asm}",
        fmt::arg("pc", cpu.pc.get()),
        fmt::arg("bytes", tmpCache),
        fmt::arg("asm", cpu::isa::decode(opcode, tmpCache)));
}

static const char* irqName(cpu::IRQ irq)
{
    switch (irq)
    {
        case cpu::IRQ::VBlank: return "VBlank";
        case cpu::IRQ::LCD:    return "LCD";
        case cpu::IRQ::Timer:  return "Timer";
        case cpu::IRQ::Serial: return "Serial";
        case cpu::IRQ::Joypad: return "Joypad";
        default:               return "unknown";
    }
}

static void printIrqs(uint8_t value)
{
    for (int i = 0; i < 5; ++i)
    {
        if ((1 << i) & value)
        {
            if (i > 0)
            {
                fmt::print(", ");
            }
            fmt::print("{}", irqName(static_cast<cpu::IRQ>(i)));
        }
    }
}

void printCpuRegs(Context& ctx)
{
    const auto& cpu = ctx.cpu;
    (void)printIrqs;
#define PRINT_REGISTER(REG, WIDTH) \
    ctx.console.writeLine("  " #REG ": {:0" #WIDTH "x}", cpu.REG.get());

    PRINT_REGISTER(af, 4);
    PRINT_REGISTER(bc, 4);
    PRINT_REGISTER(de, 4);
    PRINT_REGISTER(hl, 4);

    PRINT_REGISTER(pc, 4);
    PRINT_REGISTER(sp, 4);

    PRINT_REGISTER(ime, 1);

    PRINT_REGISTER(ie, 2);

    //fmt::print("; {{");
    //printIrqs(cpu.ie);
    //fmt::println("}}");

    PRINT_REGISTER($if, 2);

    //fmt::print("; {{");
    //printIrqs(cpu.$if);
    //fmt::println("}}");

    ctx.console.writeLine("  T-cycles:     {}", cpu.cycles);
    ctx.console.writeLine("  instructions: {}", cpu.instructions);
}

}  // namespace debugger
