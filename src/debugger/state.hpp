#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "cpu/fwd.hpp"

namespace debugger
{

struct Breakpoint
{
    uint16_t address;
    uint32_t id;
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
};

}  // namespace debugger
