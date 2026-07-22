#include "messages.hpp"

#include <imgui.h>
#include <imgui_ext.h>

namespace debugger::gui
{

static void drawMessage(Context& ctx, Message& msg, ImGuiIO& io, int i)
{
    const auto textSize = ImGui::CalcTextSize(msg.text.c_str());

    const ImVec2 pos(
        io.DisplaySize.x - textSize.x - 2 * textSize.y,
        io.DisplaySize.y - (3 + 3 * i) * textSize.y);

    ImVec4 color;

    switch (msg.severity)
    {
        case core::Severity::warning:
            color = ImGui::ColorFromHex(0xffff00);
            break;
        case core::Severity::error:
            color = ImGui::ColorFromHex(0xff0000);
            break;
        default:
            color = ImGui::ColorFromHex(0xffffff);
            break;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::SetNextWindowPos(pos, ImGuiCond_None, ImVec2(0.0, 0.0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0);
    ImGui::PushStyleVar(
        ImGuiStyleVar_Alpha,
        msg.time > ctx.gui.messageTime
        ? 1.0f - (msg.time - ctx.gui.messageTime) / float(ctx.gui.messageFadeOutTime)
        : 1.0f);

    constexpr auto messageWindowFlags
        = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoSavedSettings;

    char windowName[32];
    snprintf(windowName, sizeof(windowName), "Msg%u", i);

    auto window = ImGui::CreateWindow(
        windowName,
        nullptr,
        messageWindowFlags);

    if (window)
    {
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::Text("%s", msg.text.c_str());
        if (ImGui::WasWindowClicked(ImGuiMouseButton_Left))
        {
            msg.time = ctx.gui.messageTime + ctx.gui.messageFadeOutTime;
        }
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void drawMessages(Context& ctx)
{
    auto& messages = ctx.gui.messages;
    auto& io = ImGui::GetIO();
    int i = 0;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.1, 0.1, 0.1, 0.8});

    for (auto it = messages.begin(); it != messages.end(); ++i)
    {
        auto& msg = *it;

        if (msg.time++ >= ctx.gui.messageTime + ctx.gui.messageFadeOutTime)
        {
            messages.erase(it++);
            continue;
        }

        drawMessage(ctx, msg, io, i);

        ++it;
    }

    ImGui::PopStyleColor();
}

}  // namespace debugger::gui
