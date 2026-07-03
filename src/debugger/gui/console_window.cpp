#include "console_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "interpreter/interpreter.hpp"

namespace debugger::gui
{

void drawConsoleWindow(Context& ctx)
{
    CREATE_WINDOW("Console", ctx.gui.consoleWindow);

    auto scroll = ImGui::CreateChild("##Scroll");

    if (not scroll)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0, 1});

    for (const auto& line : ctx.console.lines)
    {
        ImGui::TextUnformatted(line.c_str());
    }

    ImGui::Text("%s ", ctx.console.prompt.c_str());
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});
    ImGui::PushItemWidth(-1);

    if (ctx.gui.commandEntered)
    {
        ImGui::SetScrollFromPosY(1.0f);
        ctx.gui.commandEntered = false;
    }

    if (ImGui::InputText("##CmdLine", ctx.gui.lineBuffer, sizeof(ctx.gui.lineBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        ctx.gui.commandEntered = true;

        std::string command(ctx.gui.lineBuffer[0] ? ctx.gui.lineBuffer : ctx.prevLine);
        ctx.gui.lineBuffer[0] = 0;

        ctx.console.writeLine("{} {}", ctx.console.prompt, command);
        auto result = interpreter::exectuteCommand(command);

        if (not result) [[unlikely]]
        {
            ctx.console.addLine(std::move(result.error()));
        }
        else
        {
            ctx.prevLine = std::move(command);
        }

        ctx.gui.focusCmdLine = true;
    }

    if (ctx.gui.focusCmdLine)
    {
        ImGui::SetKeyboardFocusHere(-1);
        ctx.gui.focusCmdLine = false;
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ctx.gui.focusCmdLine = ImGui::IsMouseClicked(ImGuiMouseButton_Left) and ImGui::IsWindowHovered();

    if (ImGui::IsWindowFocused() and ImGui::IsKeyReleased(ImGuiKey_C) and ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        ctx.console.addLine("Interrupted");
        gb.stop();
    }
}

}  // namespace debugger::gui
