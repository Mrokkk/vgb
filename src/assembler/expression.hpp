#pragma once

#include <cstdint>
#include <string_view>

#include "assembler/assembler_fwd.hpp"

namespace assembler
{

struct Expression
{
    constexpr Expression()
        : mValue(0)
    {
    }

    constexpr Expression(int32_t value)
        : mValue(value)
    {
    }

    void fromIntLiteral(Context& ctx, const std::string_view& sv, bool negative = false);

    constexpr int32_t value() const { return mValue; }

#define DEFINE_BINARY_OPERATION(NAME, OP) \
    constexpr void NAME(const Expression& lhs, const Expression& rhs) \
    { \
        mValue = lhs.mValue OP rhs.mValue; \
    }

    DEFINE_BINARY_OPERATION(add, +)
    DEFINE_BINARY_OPERATION(sub, -)
    DEFINE_BINARY_OPERATION(mult, *)
    DEFINE_BINARY_OPERATION(div, /)
    DEFINE_BINARY_OPERATION(mod, %)

    constexpr Expression& invert()
    {
        mValue *= -1;
        return *this;
    }

private:
    int32_t     mValue;
};

}  // namespace assembler
