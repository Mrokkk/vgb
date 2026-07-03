#include "lcd_window.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_ext.h>

#include "ppu.hpp"

namespace debugger::gui
{

void drawLcdWindow(Context&, unsigned int gameTextureId)
{
    auto window = ImGui::CreateWindow("LCD");

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text(ImGui::IsWindowFocused() ? "%0.1f FPS; speed: %ux" : "%0.1f FPS; speed: %ux  (click to focus)", ImGui::GetIO().Framerate, gb.speedMultiplier);
    ImGui::ImageCentered(static_cast<ImTextureID>(gameTextureId), GB_LCD_RESX, GB_LCD_RESY);
    gb.inputEnabled = ImGui::IsWindowFocused();
}

}  // namespace debugger::gui
