#include "map_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "ppu.hpp"

namespace debugger::gui
{

void drawMapWindow(Context& ctx)
{
    CREATE_WINDOW("Map", ctx.gui.mapWindow);
    ImGui::Checkbox("Show SCX/SCY window", &ctx.gui.showScxScy.get());
    auto texture = renderMap(ctx.gui.showScxScy);
    ImGui::ImageCentered(static_cast<ImTextureID>(texture.backendId), 1, 1);
}

}  // namespace debugger::gui
