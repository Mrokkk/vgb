#pragma once

#include <fmt/format.h>

#include "utils/inline.hpp"
#include "utils/string.hpp"

namespace debugger
{

struct Console
{
    Console();
    ~Console();

    template <typename ...Args>
    ALWAYS_INLINE void writeLine(fmt::format_string<Args...> fmt, Args&&... args)
    {
        addLine(fmt::format(std::move(fmt), std::forward<Args>(args)...));
    }

    void addLine(std::string line);

    void addToHistory(const std::string& command);
    std::string* prevHistoryEntry();
    std::string* nextHistoryEntry();

    void clearCurrentHistoryEntry()
    {
        historyIt = nullptr;
    }

    std::string    prompt;
    utils::Strings lines;
    utils::Strings history;
    std::string*   historyIt;
};

}  // namespace debugger
