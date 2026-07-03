#pragma once

namespace debugger::gui
{

#define ONCE_PER_X_FRAMES(INTERVAL) \
    if (ctx.gui.frameCounter % INTERVAL == 0)

#define CREATE_WINDOW(NAME, VARIABLE) \
    if (not VARIABLE) \
    { \
        return; \
    } \
    auto window = ImGui::CreateWindow(NAME, &VARIABLE.get()); \
    if (not window) [[unlikely]] \
    { \
        return; \
    }

}  // namespace debugger::gui
