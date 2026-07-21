#include "lcd_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "ppu.hpp"

namespace debugger::gui
{

void drawMinimalLcdWindow(Context&)
{
    const auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    const auto windowFlags
        = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus;

    auto window = ImGui::CreateWindow("LCD Minimal", nullptr, windowFlags);

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text("%0.1f FPS; speed: %ux", ImGui::GetIO().Framerate, gb.speedMultiplier);
    ImGui::ImageCentered(static_cast<ImTextureID>(gb.lcd.backendId), GB_LCD_RESX, GB_LCD_RESY);
    gb.inputEnabled = true;
}

void drawLcdWindow(Context&)
{
    auto window = ImGui::CreateWindow("LCD");

    if (not window) [[unlikely]]
    {
        return;
    }

    ImGui::Text(ImGui::IsWindowFocused() ? "%0.1f FPS; speed: %ux" : "%0.1f FPS; speed: %ux  (click to focus)", ImGui::GetIO().Framerate, gb.speedMultiplier);
    ImGui::ImageCentered(static_cast<ImTextureID>(gb.lcd.backendId), GB_LCD_RESX, GB_LCD_RESY);
    gb.inputEnabled = ImGui::IsWindowFocused();
}

}  // namespace debugger::gui
