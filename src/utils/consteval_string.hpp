#pragma once

#include <algorithm>
#include <cstddef>

namespace utils
{

template <size_t N>
struct ConstevalString
{
    consteval ConstevalString(const char (&string)[N])
    {
        std::copy_n(string, N, data);
    }

    char data[N];
};

}  // namespace utils
