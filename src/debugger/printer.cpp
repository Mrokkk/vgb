#include "printer.hpp"

#include <fmt/base.h>

#include "cpu/isa/decoder.hpp"
#include "cpu/isa/opcode.hpp"
#include "cpu/isa/printers.hpp"
#include "cpu/sm83.hpp"

namespace debugger
{

void printInstruction(const cpu::SM83& cpu)
{
    bool prefixed = false;

    auto tmpPc = cpu.pc;
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

    fmt::println(
        "{pc:08x}:   {bytes}     | {asm}",
        fmt::arg("pc", cpu.pc.get()),
        fmt::arg("bytes", tmpCache),
        fmt::arg("asm", cpu::isa::decode(opcode, tmpCache)));
}

void printCpuRegs(const cpu::SM83& cpu)
{
#define PRINT_REGISTER(REG, WIDTH) \
    fmt::println("  " #REG ": {:0" #WIDTH "x}", cpu.REG.get());

    PRINT_REGISTER(af, 4);
    PRINT_REGISTER(bc, 4);
    PRINT_REGISTER(de, 4);
    PRINT_REGISTER(hl, 4);

    PRINT_REGISTER(pc, 4);
    PRINT_REGISTER(sp, 4);
    fmt::println("  f:  {{c: {}, h: {}, n: {}, z: {}}}",
        cpu.f.c, cpu.f.h, cpu.f.n, cpu.f.z);

    PRINT_REGISTER(ime, 1);
    PRINT_REGISTER(ie, 2);
    PRINT_REGISTER($if, 2);
}

}  // namespace debugger
