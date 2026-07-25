#include "console_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "game_boy.hpp"
#include "interpreter/interpreter.hpp"

namespace debugger::gui
{

static int inputCallback(ImGuiInputTextCallbackData* data)
{
    auto& ctx = *static_cast<Context*>(data->UserData);

    switch (data->EventFlag)
    {
        case ImGuiInputTextFlags_CallbackCompletion:
            break;

        case ImGuiInputTextFlags_CallbackHistory:
        {
            if (ctx.console.history.empty())
            {
                return 0;
            }

            auto it = ctx.console.historyIt;

            if (data->EventKey == ImGuiKey_UpArrow)
            {
                it = ctx.console.prevHistoryEntry();
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                it = ctx.console.nextHistoryEntry();
            }

            data->DeleteChars(0, data->BufTextLen);
            if (it)
            {
                data->InsertChars(0, it->c_str());
            }

            break;
        }
    }

    return 0;
}

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

    if (gb.state != GameBoy::State::Running)
    {
        ImGui::Text("%s ", ctx.console.prompt.c_str());
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});
        ImGui::PushItemWidth(-1);

        if (ctx.gui.commandEntered)
        {
            ImGui::SetScrollFromPosY(1.0f);
            ctx.gui.commandEntered = false;
        }

        constexpr int inputFlags
            = ImGuiInputTextFlags_EnterReturnsTrue
            | ImGuiInputTextFlags_CallbackHistory
            | ImGuiInputTextFlags_CallbackCompletion;

        if (ImGui::InputText("##CmdLine", ctx.gui.lineBuffer, sizeof(ctx.gui.lineBuffer), inputFlags, &inputCallback, &ctx))
        {
            ctx.gui.commandEntered = true;

            std::string command(ctx.gui.lineBuffer[0] ? ctx.gui.lineBuffer : ctx.prevLine);
            ctx.gui.lineBuffer[0] = 0;

            if (not command.empty())
            {
                ctx.console.addToHistory(command);
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
            }
            else
            {
                ctx.console.writeLine("{}", ctx.console.prompt);
            }

            ctx.console.clearCurrentHistoryEntry();

            ctx.gui.focusCmdLine = true;
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::PopItemWidth();
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##CmdLine", ctx.gui.lineBuffer, sizeof(ctx.gui.lineBuffer), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar(2);

    if (ctx.gui.focusCmdLine)
    {
        ImGui::SetKeyboardFocusHere(-1);
        ctx.gui.focusCmdLine = false;
    }

    ctx.gui.focusCmdLine = ImGui::IsMouseClicked(ImGuiMouseButton_Left) and ImGui::IsWindowHovered();

    if (ImGui::IsWindowFocused() and ImGui::IsKeyReleased(ImGuiKey_C) and ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
    {
        if (gb.state == GameBoy::State::Running)
        {
            ctx.console.writeLine("^C");
        }
        else
        {
            ctx.console.writeLine("{} {}^C", ctx.console.prompt, ctx.gui.lineBuffer[0] ? ctx.gui.lineBuffer : "");
        }
        ctx.gui.focusCmdLine = true;
        gb.stop();
    }
}

}  // namespace debugger::gui
