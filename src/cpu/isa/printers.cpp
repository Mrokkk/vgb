#include "printers.hpp"

#include <fmt/base.h>

#include "cpu/isa/opcode.hpp"

fmt::format_context::iterator fmt::formatter<cpu::isa::InstructionData>::format(const cpu::isa::InstructionData& data, format_context& ctx) const
{
    auto it = ctx.out();
    uint16_t i;
    for (i = 0; i < data.bytes; ++i)
    {
        it = fmt::format_to(it, "{:02x} ", data.data[i]);
    }
    for (; i < sizeof(data.data); ++i)
    {
        it = fmt::format_to(it, "   ", data.data[i]);
    }
    return it;
}
