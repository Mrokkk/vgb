#include "printers.hpp"

#include <string>

#include <fmt/format.h>

#include "src/cpu/printers.hpp"

#define BOLD  "\033[1m"
#define RED   "\033[31;1m"
#define RESET "\033[0m"

template <typename T>
ALWAYS_INLINE constexpr T numberOfDigits(T x)
{
    return x > 0 ? (T)std::log10((double)x) + 1 : 1;
}

std::string TestStringConverter<assembler::MaybeRom>::convert(const assembler::MaybeRom& value)
{
    if (value)
    {
        return "Valid ROM";
    }

    std::string out;

    for (const auto& e : value.error())
    {
        const auto& loc = e.location;
        const auto& msg = e.message;

        if (loc)
        {
            out += fmt::format(
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
            out += fmt::format(RED "error:" RESET" {}\n", msg);
        }
    }

    return out;
}

std::string TestStringConverter<cpu::Register16>::convert(cpu::Register16 value)
{
    return fmt::format("{:#04x}", value.get());
}

std::string TestStringConverter<cpu::Register8>::convert(cpu::Register8 value)
{
    return fmt::format("{:#02x}", value.get());
}

std::string TestStringConverter<cpu::Exception>::convert(cpu::Exception value)
{
    return fmt::format("{}", value);
}
