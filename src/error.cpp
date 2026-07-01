#include "error.hpp"

#include <cstdio>

#include <fmt/base.h>

std::unexpected<std::string> error(std::string msg)
{
    return std::unexpected(std::move(msg));
}

namespace detail
{

[[noreturn]] void assertionFailed(const char* file, size_t line, const char* func, std::string_view message)
{
    fmt::println(stderr, "Assertion failed: {}:{}:{}: {}",
        file,
        line,
        func,
        message);
    std::abort();
}

}  // namespace detail
