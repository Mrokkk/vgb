#include "config_window.hpp"

#include <cstring>

#include <imgui.h>
#include <imgui_ext.h>

#include "sys/platform.hpp"

namespace debugger::gui
{

static void setFont(Context& ctx, ImGuiIO& io, const sys::Font& font, const sys::FontStyle& fontStyle)
{
    if (io.FontDefault != ctx.gui.defaultFont)
    {
        io.Fonts->RemoveFont(io.FontDefault);
    }
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontStyle.path.c_str());
    ctx.gui.fontFamily = font.family;
    ctx.gui.fontStyle = fontStyle.name;
    ctx.gui.currentFont = &font;
    ctx.gui.currentFontStyle = &fontStyle;
}

static void setDefaultFont(Context& ctx, ImGuiIO& io)
{
    io.FontDefault = static_cast<ImFont*>(ctx.gui.defaultFont);
    ctx.gui.fontFamily = "(default)";
    ctx.gui.fontStyle = "(default)";
    ctx.gui.currentFont = nullptr;
    ctx.gui.currentFontStyle = nullptr;
}

static void drawViewConfig(Context& ctx, ImGuiIO& io)
{
    ImGui::SeparatorText("Font");

    if (auto _ = ImGui::CreateCombo("Font family", ctx.gui.fontFamily.get().c_str(), ImGuiComboFlags_HeightLargest))
    {
        if (ImGui::Selectable("(default)", ctx.gui.fontFamily.get() == "(default)"))
        {
            setDefaultFont(ctx, io);
        }

        int i = 0;
        for (const auto& font : ctx.gui.fonts)
        {
            ImGui::PushID(i++);
            if (ImGui::Selectable(font.family.c_str(), &font == ctx.gui.currentFont))
            {
                setFont(ctx, io, font, font.styles[0]);
            }
            ImGui::PopID();
        }
    }
    if (auto _ = ImGui::CreateCombo("Font style", ctx.gui.fontStyle.get().c_str()))
    {
        if (not ctx.gui.currentFont)
        {
            ImGui::Selectable("(default)", true);
        }
        else
        {
            int i = 0;
            for (const auto& style : ctx.gui.currentFont->styles)
            {
                ImGui::PushID(i++);
                if (ImGui::Selectable(style.name.c_str(), &style == ctx.gui.currentFontStyle))
                {
                    setFont(ctx, io, *ctx.gui.currentFont, style);
                }
                ImGui::PopID();
            }
        }
    }
    if (ImGui::InputFloat("Font size", &ctx.gui.fontSize.get(), 0.1f, 20.0f, "%.1f"))
    {
        auto& style = ImGui::GetStyle();
        style._NextFrameFontSizeBase = ctx.gui.fontSize;
    }
}

static void updatePalette(const ImVec4* colors)
{
    uint32_t palette[4] = {
        ImGui::GetColorU32(colors[0]) & 0xffffff,
        ImGui::GetColorU32(colors[1]) & 0xffffff,
        ImGui::GetColorU32(colors[2]) & 0xffffff,
        ImGui::GetColorU32(colors[3]) & 0xffffff,
    };
    sys::platform.renderer->setPalette(palette);
}

static void readPalette(ImVec4* colors)
{
    auto palette = sys::platform.renderer->getPalette();

    for (size_t i = 0; i < 4; ++i)
    {
        colors[i] = ImGui::ColorFromHex(palette[i]);
    }
}

static void drawGameBoyConfig(Context&)
{
    static ImVec4 colors[4];
    static ImVec4 originalColors[4];
    static bool initialized = false;
    static bool dirty = false;

    if (not initialized)
    {
        readPalette(originalColors);
        memcpy(colors, originalColors, sizeof(colors));
        initialized = true;
    }

    ImGui::SeparatorText("Palette");

    for (int i = 0; i < 4; ++i)
    {
        ImGui::PushID(i);
        ImGui::Text("Color %i", i);
        ImGui::SameLine();
        if (ImGui::ColorEdit3("##color", (float*)&colors[i], ImGuiColorEditFlags_NoInputs))
        {
            dirty = memcmp(colors, originalColors, sizeof(colors)) != 0;
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Apply", dirty))
    {
        updatePalette(colors);
        memcpy(originalColors, colors, sizeof(colors));
        dirty = false;
    }

    if (dirty)
    {
        if (ImGui::SameLineButton("Reset"))
        {
            memcpy(colors, originalColors, sizeof(colors));
            dirty = false;
        }
    }
}

void drawConfigWindow(Context& ctx)
{
    if (not ctx.gui.configWindow)
    {
        return;
    }

    ImGui::OpenPopup("Configuration");

    auto& io = ImGui::GetIO();

    constexpr auto configWindowFlags
        = ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowSize(ImVec2{500, 300});

    auto popup = ImGui::CreatePopupModal("Configuration", &ctx.gui.configWindow, configWindowFlags);

    if (not popup)
    {
        return;
    }

    if (auto _ = ImGui::CreateTabBar("Config"))
    {
        if (auto _ = ImGui::CreateTabItem("View"))
        {
            drawViewConfig(ctx, io);
        }
        if (auto _ = ImGui::CreateTabItem("GameBoy"))
        {
            drawGameBoyConfig(ctx);
        }
    }
}

}  // namespace debugger::gui
