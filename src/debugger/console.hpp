#pragma once

#include <fmt/format.h>

#include "utils/inline.hpp"
#include "utils/string.hpp"

namespace debugger
{

struct Console
{
    template <typename ...Args>
    ALWAYS_INLINE void writeLine(fmt::format_string<Args...> fmt, Args&&... args)
    {
        lines.push_back(fmt::format(std::move(fmt), std::forward<Args>(args)...));
    }

    ALWAYS_INLINE void addLine(std::string line)
    {
        lines.push_back(std::move(line));
    }

    std::string    prompt;
    utils::Strings lines;
};

}  // namespace debugger
