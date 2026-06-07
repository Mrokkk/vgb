#pragma once

#include <string>

#include <fmt/format.h>

namespace fmt
{

template <typename ...Args>
std::string format_to_string(format_string<Args...> fmt, Args&&... args)
{
    std::string tmp;
    format_to(std::back_inserter(tmp), fmt, std::forward<Args>(args)...);
    return tmp;
}

}  // namespace fmt
