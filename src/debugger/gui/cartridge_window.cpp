#include "cartridge_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "utils/units.hpp"

namespace debugger::gui
{

void drawCartridgeWindow(Context& ctx)
{
    CREATE_WINDOW("Cartridge", ctx.gui.cartridgeWindow);

    ImGui::Text("Path: %s", gb.config.cartridgePath.c_str());
    ImGui::Text("Title: %s", gb.cartridge.getTitle());
    ImGui::Text("Type: %s", gb.cartridge.getType());

    auto romSize = utils::humanReadable(gb.cartridge.getRomSize());
    auto ramSize = utils::humanReadable(gb.cartridge.getRamSize());

    ImGui::Text("ROM size: %zu %s", romSize.value, romSize.unit);
    ImGui::Text("RAM size: %zu %s", ramSize.value, ramSize.unit);
}

}  // namespace debugger::gui
