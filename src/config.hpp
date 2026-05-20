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
    bool        skipBootRom;
    bool        useDebugger;
    VideoConfig videoConfig;
};
