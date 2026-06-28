#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>

#include <fmt/fmt_ext.h>

#include "cpu/fwd.hpp"
#include "debugger/console.hpp"
#include "debugger/games/game.hpp"
#include "debugger/symbols_map.hpp"
#include "game_boy.hpp"
#include "severity.hpp"

namespace debugger
{

struct Breakpoint final
{
    uint16_t address;
    uint32_t id;
};

struct Message final
{
    const Severity    severity;
    unsigned          time;
    const std::string text;
};

struct GUI final
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
    bool        callstackWindow;
    size_t      counter;
    float       ips;
    float       mhz;
    float       sumIps;
    float       sumMhz;
    uint64_t    prevInstructions;
    uint64_t    prevCycles;
    unsigned    messageTime;
    unsigned    messageFadeOutTime;

    char lineBuffer[256];
    char addrBuffer[32];
    char ioFilterBuffer[32];

    std::string iniPath;

    std::list<Message> messages;
};

struct Context final
{
    GameBoy&    gb;
    cpu::SM83&  cpu;
    int         prevBreakpoint;
    std::string prevLine;

    std::map<uint16_t, Breakpoint> watchpoints;
    std::map<uint16_t, Breakpoint> breakpoints;

    Console console;
    GUI gui;
    SymbolsMap symbols;

    games::GamePtr game;
};

}  // namespace debugger
