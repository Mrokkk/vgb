#pragma once

#include <cassert> // IWYU pragma: export
#include <cstddef>
#include <expected>
#include <string_view>
#include <utility>

#include <fmt/format.h>

std::unexpected<std::string> error(std::string msg);

template <typename ...Args>
auto error(fmt::format_string<Args...> fmt, Args&&... args)
{
    return std::unexpected(fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...));
}

#ifndef NDEBUG
#define assertFormat(expr, ...) \
    ({ \
        if (not static_cast<bool>(expr)) [[unlikely]] \
        { \
            ::detail::assertionFailed(__FILE__, __LINE__, __func__, __VA_ARGS__); \
        } \
        void(0); \
    })
#else
#define assertFormat(...) void(0)
#endif

namespace detail
{

[[noreturn]] void assertionFailed(const char* file, size_t line, const char* func, std::string_view message);

template <typename ...Args>
[[noreturn]] void assertionFailed(const char* file, size_t line, const char* func, fmt::format_string<Args...> fmt, Args&&... args)
{
    assertionFailed(file, line, func, std::string_view(fmt::format(fmt, std::forward<Args>(args)...)));
}

}  // namespace detail
