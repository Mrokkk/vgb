#include "console.hpp"

namespace debugger
{

Console::Console()
    : prompt("(vgb)")
    , history{}
    , historyIt(nullptr)
{
}

Console::~Console() = default;

void Console::addLine(std::string line)
{
    lines.push_back(std::move(line));
}

void Console::addToHistory(const std::string& command)
{
    if (not history.empty() and history.back() == command)
    {
        return;
    }
    history.emplace_back(command);
}

std::string* Console::prevHistoryEntry()
{
    if (historyIt)
    {
        if (historyIt != &history.front())
        {
            --historyIt;
        }
    }
    else
    {
        historyIt = &history.back();
    }
    return historyIt;
}

std::string* Console::nextHistoryEntry()
{
    if (historyIt)
    {
        if (historyIt != &history.back())
        {
            ++historyIt;
        }
        else
        {
            historyIt = nullptr;
        }
    }
    return historyIt;
}

}  // namespace debugger
