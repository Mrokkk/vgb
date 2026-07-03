#include "game_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"

namespace debugger::gui
{

void drawGameWindow(Context& ctx)
{
    CREATE_WINDOW("Game", ctx.gui.gameWindow);

    if (not ctx.game)
    {
        ImGui::TextDisabled("Not supported game");
        return;
    }

    ctx.game->drawUi();
}

}  // namespace debugger::gui
