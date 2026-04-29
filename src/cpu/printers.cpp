#include "printers.hpp"

#include <fmt/base.h>

#include "cpu/exception.hpp"
#include "cpu/sm83.hpp"

fmt::format_context::iterator fmt::formatter<cpu::Exception>::format(const cpu::Exception& exc, format_context& ctx) const
{
    switch (exc.type)
    {
        case cpu::Exception::NotImplemented:
            return format_to(ctx.out(), "instruction not implemented: {:02x}", exc.value);
        case cpu::Exception::InvalidOpcode:
            return format_to(ctx.out(), "invalid opcode: {:02x}", exc.value);
        case cpu::Exception::SegmentationFault:
            return format_to(ctx.out(), "segmentation fault caused by {} address {:04x}", exc.segmentationFault.write ? "write to" : "read from", exc.segmentationFault.addr);
        case cpu::Exception::InfiniteLoop:
            return format_to(ctx.out(), "infinite loop detected");
        case cpu::Exception::UserInterruption:
            return format_to(ctx.out(), "user interruption");
        case cpu::Exception::Halt:
            return format_to(ctx.out(), "CPU is halted");
        default:
            return format_to(ctx.out(), "unknown exception: {}", +exc.type);
    }
}

fmt::format_context::iterator fmt::formatter<cpu::SM83>::format(const cpu::SM83& cpu, format_context& ctx) const
{
    auto it = ctx.out();

#define PRINT_REGISTER(REG, WIDTH) \
    it = format_to(it, "  " #REG ": {:0" #WIDTH "x}; ", cpu.REG.get());

    PRINT_REGISTER(af, 4);
    PRINT_REGISTER(bc, 4);
    PRINT_REGISTER(de, 4);
    PRINT_REGISTER(hl, 4);

    it = format_to(it, "\n");

    PRINT_REGISTER(pc, 4);
    PRINT_REGISTER(sp, 4);
    it = format_to(it, "\n  f:  {{c: {}, h: {}, n: {}, z: {}}}",
        cpu.f.c, cpu.f.h, cpu.f.n, cpu.f.z);

    it = format_to(it, "\n");

    PRINT_REGISTER(ime, 1);
    PRINT_REGISTER(ie, 2);
    PRINT_REGISTER($if, 2);

    return it;
}
