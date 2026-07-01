#pragma once

#include <string>

enum class Mode
{
    Headless,
    Minimal,
    Debugger
};

struct Config final
{
    std::string cartridgePath;
    std::string cartridgeRamPath;
    bool        skipBootRom;
    bool        useSupervision;
    Mode        mode;
};
