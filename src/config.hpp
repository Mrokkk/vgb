#pragma once

#include <string>

enum class VideoConfig
{
    Graphical,
    Headless,
};

struct Config
{
    std::string cartridgePath;
    std::string cartridgeRamPath;
    bool        skipBootRom;
    bool        useDebugger;
    VideoConfig videoConfig;
};
