#pragma once

#include <list>
#include <string>

#include "core/severity.hpp"
#include "core/ini_serializer.hpp"
#include "sys/font.hpp"

class MemoryEditor;

namespace debugger::gui
{

struct Message final
{
    const core::Severity severity;
    unsigned             time;
    const std::string    text;
};

struct GUI final
{
    bool                   commandEntered;
    INI_SAVED(bool,        minimalMode);
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
    bool*                  memEditorWindow;
    bool                   configWindow;
    size_t                 frameCounter;
    float                  ips;
    float                  mhz;
    float                  sumIps;
    float                  sumMhz;
    uint64_t               prevInstructions;
    uint64_t               prevCycles;
    void*                  defaultFont;
    const sys::Font*       currentFont;
    const sys::FontStyle*  currentFontStyle;
    sys::Fonts             fonts;
    MemoryEditor*          memEditor;
    char                   lineBuffer[256];
    char                   addrBuffer[32];
    char                   ioFilterBuffer[32];
    std::string            iniPath;
    std::list<Message>     messages;
};

}  // namespace debugger::gui
