#include "expression.hpp"

#include <charconv>

#include <fmt/base.h>

#include "assembler/glue.hpp"

namespace assembler
{

void Expression::fromIntLiteral(Context& ctx, const std::string_view& sv, bool negative)
{
    int base = 10;
    const char* start = sv.begin();
    if (sv.starts_with("0x") or sv.starts_with("0X"))
    {
        base = 16;
        start += 2;
    }
    else if (sv.starts_with("$"))
    {
        base = 16;
        start += 1;
    }
    else if (sv.starts_with("0o") or sv.starts_with("0O"))
    {
        base = 8;
        start += 2;
    }
    else if (sv.starts_with("&"))
    {
        base = 8;
        start += 1;
    }
    else if (sv.starts_with("0b") or sv.starts_with("0B"))
    {
        base = 2;
        start += 2;
    }
    else if (sv.starts_with("%"))
    {
        base = 2;
        start += 1;
    }
    auto result = std::from_chars(start, sv.end(), mValue, base);
    if (result.ec == std::errc::invalid_argument) [[unlikely]]
    {
        reportError(ctx, "internal error: not a number: {}", sv);
        return;
    }
    else if (result.ec == std::errc::result_out_of_range) [[unlikely]]
    {
        reportError(ctx, "number out of valid range: {}", sv);
        return;
    }
    if (negative)
    {
        mValue *= -1;
    }
}

}  // namespace assembler
