#include "printers.hpp"

#include <cstdio>
#include <string>

#include <fmt/format.h>

#include "src/cpu/printers.hpp"

namespace doctest
{

#define BOLD  "\033[1m"
#define RED   "\033[31;1m"
#define RESET "\033[0m"

template <typename T>
ALWAYS_INLINE constexpr T numberOfDigits(T x)
{
    return x > 0 ? (T)std::log10((double)x) + 1 : 1;
}

String StringMaker<assembler::MaybeRom>::convert(const assembler::MaybeRom& value)
{
    if (value)
    {
        return String("Valid ROM");
    }

    std::string str("\n");
    str.reserve(1024);

    for (const auto& e : value.error())
    {
        const auto& loc = e.location;
        const auto& msg = e.message;

        if (loc)
        {
            str += fmt::format(
                BOLD "{fileName}:{lineNo}:{pos}: " RESET RED "error:" RESET" {msg}\n"
                "{lineNo} | {line}\n"
                "{empty:<{digits}} | {marker:>{pos}}\n",
                fmt::arg("fileName", loc->fileName), fmt::arg("lineNo", loc->lineNo), fmt::arg("pos", loc->pos), fmt::arg("msg", msg),
                fmt::arg("line", loc->line),
                fmt::arg("empty", ""), fmt::arg("digits", numberOfDigits(loc->lineNo)),
                fmt::arg("marker", "^"));
        }
        else
        {
            str += fmt::format(RED "error:" RESET" {}\n", msg);
        }
    }

    return String(str.data(), str.size());
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
    auto str = fmt::format("{}", value);
    return String(str.data(), str.size());
}

}  // namespace doctest
