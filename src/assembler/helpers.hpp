#pragma once

#include <expected> // IWYU pragma: keep
#include <utility>

#include <fmt/format.h> // IWYU pragma: keep

namespace assembler
{

template <typename T, typename U>
auto findValue(const T& map, U&& value) -> const typename T::mapped_type*
{
    auto it = map.find(std::forward<U>(value));
    if (it == map.end()) [[unlikely]]
    {
        return nullptr;
    }
    return &it->second;
}

#define REQUIRE(CONDITION, ...) \
    do \
    { \
        if (not (CONDITION)) [[unlikely]] \
        { \
            return std::unexpected(fmt::format(__VA_ARGS__)); \
        } \
    } \
    while (0)

#define REQUIRE_FALSE(CONDITION, ...) \
    REQUIRE(not (CONDITION), __VA_ARGS__)

}  // namespace assembler
