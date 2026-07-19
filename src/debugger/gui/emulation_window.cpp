#include "emulation_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "core/logger.hpp"
#include "debugger/gui/helpers.hpp"
#include "interpreter/interpreter.hpp"
#include "save_manager.hpp"

namespace debugger::gui
{

static void execute(Context&, std::string command)
{
    auto result = interpreter::exectuteCommand(std::move(command));

    if (not result) [[unlikely]]
    {
        core::logger.error().buffer() = std::move(result.error());
    }
}

void drawEmulationWindow(Context& ctx)
{
    if (ImGui::IsKeyReleased(ImGuiKey_F5))
    {
        SaveManager::quickSave();
    }
    else if (ImGui::IsKeyReleased(ImGuiKey_F9))
    {
        SaveManager::quickLoad();
    }

    CREATE_WINDOW("Emulation", ctx.gui.emulationWindow);

    ImGui::SeparatorText("Emulation");
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            if (ImGui::Button("Start"))
            {
                gb.start();
            }
        }
        else
        {
            if (ImGui::Button("Stop"))
            {
                gb.stop();
            }
        }

        if (ImGui::SameLineButton("Reset"))
        {
            execute(ctx, "reset");
        }

        ImGui::SliderInt("Speed", reinterpret_cast<int*>(&gb.speedMultiplier), 1, 20);

        if (ImGui::Button("Step"))
        {
            execute(ctx, "step");
        }
    }

    ImGui::SeparatorText("Save RAM");
    {
        if (ImGui::Button("Save", gb.cartridge.isRamDirty()))
        {
            gb.saveRam();
        }
    }

    ImGui::SeparatorText("Saves");
    {
        ImGui::Text("Saves directory: %s", SaveManager::getSaveDir().c_str());
        if (ImGui::Button("Quick save (F5)"))
        {
            SaveManager::quickSave();
        }
        if (ImGui::Button("Quick load (F9)"))
        {
            SaveManager::quickLoad();
        }
        ImGui::SeparatorText("Saves");
        {
            unsigned i = 0;
            for (const auto& save : SaveManager::getSaves())
            {
                ImGui::Text("%u: %s", i, save.name.c_str());
            }
        }
    }
}

}  // namespace debugger::gui
