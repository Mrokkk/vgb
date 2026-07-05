#pragma once

#include <string_view> // IWYU pragma: keep

#include "test/tools/compiler.hpp"

namespace test::tools
{

[[noreturn]] void throwImpl(const char* fmt, ...) FORMAT(printf, 1, 2);

#define ASSERT_THROW(CONDITION, FMT, ...) \
    do \
    { \
        if (not (CONDITION)) [[unlikely]] \
        { \
            std::string_view sv(__builtin_FILE()); \
            auto pos = sv.find("test"); \
            if (pos != sv.npos) [[likely]] \
            { \
                sv.remove_prefix(pos); \
            } \
            ::test::tools::throwImpl("%s:%u: %s: " FMT, sv.data(), __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__); \
        } \
    } \
    while (0)

}  // namespace test::tools
