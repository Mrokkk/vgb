#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <fmt/fmt_ext.h>

#include "cpu/fwd.hpp"
#include "debugger/console.hpp"
#include "debugger/games/game.hpp"
#include "game_boy.hpp"

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
    bool        emulationWindow;
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
    bool        disassemblyWindow;

    char lineBuffer[256];
    char addrBuffer[32];
    char ioFilterBuffer[32];
};

struct Context
{
    GameBoy&    gb;
    cpu::SM83&  cpu;
    int         prevBreakpoint;
    std::string prevLine;

    std::map<uint16_t, Breakpoint> watchpoints;
    std::map<uint16_t, Breakpoint> breakpoints;

    Console console;
    GUI gui;

    games::GamePtr game;
};

}  // namespace debugger
