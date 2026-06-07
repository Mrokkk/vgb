#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <fmt/fmt_ext.h>

#include "cpu/fwd.hpp"
#include "debugger/games/game.hpp"
#include "utils/inline.hpp"

namespace debugger
{

struct Breakpoint
{
    uint16_t address;
    uint32_t id;
};

struct GUI
{
    bool        commandEntered;
    bool        cartridgeWindow;
    bool        cpuWindow;
    bool        consoleWindow;
    bool        mapWindow;
    bool        showScxScy;
    bool        styleEditorWindow;
    bool        ioWindow;
    bool        gameWindow;
    bool        focusCmdLine;
    bool        demoWindow;
    bool        logWindow;

    char lineBuffer[256];
    char addrBuffer[32];
    char ioFilterBuffer[32];
};

struct State
{
    cpu::SM83&  cpu;
    bool        stopped;
    bool        printRegs;
    int         prevBreakpoint;
    std::string prompt;
    std::string prevLine;

    std::map<uint16_t, Breakpoint> watchpoints;
    std::map<uint16_t, Breakpoint> breakpoints;

    std::vector<std::string> consoleLines;

    GUI gui;

    games::GamePtr game;
};

template <typename ...Args>
ALWAYS_INLINE void logToConsole(State& state, fmt::format_string<Args...> fmt, Args&&... args)
{
    state.consoleLines.push_back(fmt::format_to_string(std::move(fmt), std::forward<Args>(args)...));
}

}  // namespace debugger
