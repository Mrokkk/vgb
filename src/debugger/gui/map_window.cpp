#include "map_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "sys/platform.hpp"

namespace debugger::gui
{

void drawMapWindow(Context& ctx)
{
    CREATE_WINDOW("Map", ctx.gui.mapWindow);
    ImGui::Checkbox("Show SCX/SCY window", &ctx.gui.showScxScy.get());
    ImGui::ImageCentered(static_cast<ImTextureID>(sys::platform.renderer->renderMap(ctx.gui.showScxScy)), 1, 1);
}

}  // namespace debugger::gui
