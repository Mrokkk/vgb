#pragma once

#include <cstdint>

#include "utils/inline.hpp"

namespace cpu
{

#define DEFINE_OPERATION(OP) ALWAYS_INLINE T operator OP(T val) { return mValue OP val; }

template <typename T>
struct Register
{
    ALWAYS_INLINE operator T() const { return mValue; }
    ALWAYS_INLINE T get()      const { return mValue; }

    ALWAYS_INLINE T operator++(int) { return mValue++; }
    ALWAYS_INLINE T operator++()    { return ++mValue; }
    ALWAYS_INLINE T operator--(int) { return mValue--; }
    ALWAYS_INLINE T operator--()    { return --mValue; }

    ALWAYS_INLINE bool operator==(const Register& rhs) const { return mValue == rhs.mValue; }

    DEFINE_OPERATION(=)
    DEFINE_OPERATION(+=)
    DEFINE_OPERATION(-=)
    DEFINE_OPERATION(^=)
    DEFINE_OPERATION(|=)
    DEFINE_OPERATION(&=)

private:
    T mValue;
};

struct Register8 : Register<uint8_t>
{
    using Register::operator=;
};

struct Register16 : Register<uint16_t>
{
    using Register::operator=;
};

}  // namespace cpu
