#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>

#include <fmt/fmt_ext.h>

#include "core/ini_serializer.hpp"
#include "core/severity.hpp"
#include "cpu/fwd.hpp"
#include "debugger/console.hpp"
#include "debugger/games/game.hpp"
#include "debugger/symbols_map.hpp"
#include "game_boy.hpp"
#include "sys/font.hpp"

namespace debugger
{

struct Breakpoint final
{
    uint16_t address;
    uint32_t id;
};

struct Message final
{
    const core::Severity severity;
    unsigned             time;
    const std::string    text;
};

struct GUI final
{
    INI_SAVED(bool,        commandEntered);
    INI_SAVED(bool,        emulationWindow);
    INI_SAVED(bool,        cartridgeWindow);
    INI_SAVED(bool,        cpuWindow);
    INI_SAVED(bool,        consoleWindow);
    INI_SAVED(bool,        mapWindow);
    INI_SAVED(bool,        showScxScy);
    INI_SAVED(bool,        styleEditorWindow);
    INI_SAVED(bool,        ioWindow);
    INI_SAVED(bool,        gameWindow);
    INI_SAVED(bool,        focusCmdLine);
    INI_SAVED(bool,        demoWindow);
    INI_SAVED(bool,        logWindow);
    INI_SAVED(bool,        disassemblyWindow);
    INI_SAVED(bool,        callstackWindow);
    INI_SAVED(bool,        systemStatsWindow);
    INI_SAVED(uint32_t,    messageTime);
    INI_SAVED(uint32_t,    messageFadeOutTime);
    INI_SAVED(std::string, fontFamily);
    INI_SAVED(std::string, fontStyle);
    INI_SAVED(float,       fontSize);

    bool                  configWindow;
    size_t                frameCounter;
    float                 ips;
    float                 mhz;
    float                 sumIps;
    float                 sumMhz;
    uint64_t              prevInstructions;
    uint64_t              prevCycles;
    void*                 defaultFont;
    const sys::Font*      currentFont;
    const sys::FontStyle* currentFontStyle;
    sys::Fonts            fonts;

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
