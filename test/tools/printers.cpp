#include "printers.hpp"

#include <cstdio>
#include <string>

#include <fmt/fmt_ext.h>

#include "src/cpu/printers.hpp"

namespace doctest
{

String StringMaker<cpu::isa::MaybeRom>::convert(const cpu::isa::MaybeRom& value)
{
    if (not value)
    {
        return String(value.error().data(), value.error().size());
    }
    return "Valid ROM";
}

String StringMaker<cpu::Register16>::convert(cpu::Register16 value)
{
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%#04x", value.get());
    return buffer;
}

String StringMaker<cpu::Register8>::convert(cpu::Register8 value)
{
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%#02x", value.get());
    return buffer;
}

String StringMaker<cpu::Exception>::convert(cpu::Exception value)
{
    auto str = fmt::format_to_string("{}", value);
    return String(str.data(), str.size());
}

}  // namespace doctest
