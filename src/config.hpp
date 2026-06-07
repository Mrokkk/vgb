#pragma once

#include <string>

enum class VideoConfig
{
    Graphical,
    Headless,
};

struct Config final
{
    std::string cartridgePath;
    std::string cartridgeRamPath;
    bool        skipBootRom;
    bool        useDebugger;
    bool        useSupervision;
    VideoConfig videoConfig;
};
