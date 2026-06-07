#include "printers.hpp"

#include <fmt/base.h>

#include "cpu/exception.hpp"
#include "cpu/sm83.hpp"

using namespace cpu;

fmt::format_context::iterator fmt::formatter<Exception>::format(const Exception& exc, format_context& ctx) const
{
    switch (exc.type)
    {
        case Exception::NotImplemented:
            return format_to(ctx.out(), "instruction not implemented: {:02x}", exc.value);
        case Exception::InvalidOpcode:
            return format_to(ctx.out(), "invalid opcode: {:02x}", exc.value);
        case Exception::SegmentationFault:
            return exc.segmentationFault.write
                ? format_to(ctx.out(), "segmentation fault caused by writing {:02x} to address {:04x}", exc.segmentationFault.value, exc.segmentationFault.addr)
                : format_to(ctx.out(), "segmentation fault caused by reading from address {:04x}", exc.segmentationFault.addr);
        case Exception::InfiniteLoop:
            return format_to(ctx.out(), "infinite loop detected");
        case Exception::UserInterruption:
            return format_to(ctx.out(), "user interruption");
        case Exception::Halt:
            return format_to(ctx.out(), "CPU is halted");
        default:
            return format_to(ctx.out(), "unknown exception: {}", +exc.type);
    }
}

fmt::format_context::iterator fmt::formatter<SM83>::format(const SM83& cpu, format_context& ctx) const
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
