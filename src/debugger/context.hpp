#pragma once

#include <cstdint>
#include <map>
#include <string>

#include <fmt/fmt_ext.h>

#include "cpu/fwd.hpp"
#include "debugger/console.hpp"
#include "debugger/games/game.hpp"
#include "debugger/gui/gui.hpp"
#include "debugger/symbols_map.hpp"
#include "game_boy.hpp"

namespace debugger
{

struct Breakpoint final
{
    uint16_t address;
    uint32_t id;
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
    gui::GUI gui;
    SymbolsMap symbols;

    games::GamePtr game;
};

}  // namespace debugger
