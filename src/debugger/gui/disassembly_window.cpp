#include "disassembly_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "cpu/isa/decoder.hpp"
#include "debugger/gui/helpers.hpp"

namespace debugger::gui
{

void drawDisassemblyWindow(Context& ctx)
{
    CREATE_WINDOW("Disassembly", ctx.gui.disassemblyWindow);

    if (ctx.gb.state == GameBoy::State::Running)
    {
        ImGui::TextDisabled("--");
        return;
    }

    auto disassembleCtx = cpu::isa::DisassembleContext::create(ctx.cpu, ctx.cpu.pc);

    constexpr auto tableFlags
        = ImGuiTableFlags_Borders
        | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_NoBordersInBody;

    if (auto table = ImGui::CreateTable("Disassembly", 3, tableFlags))
    {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Bytes");
        ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_NoHide);
        ImGui::TableHeadersRow();

        for (int i = 0; i < 10; ++i)
        {
            auto addr = disassembleCtx.pc;
            cpu::isa::disassemble(disassembleCtx);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%04x", addr);
            ImGui::TableNextColumn();
            for (size_t i = 0; i < disassembleCtx.data.bytes; ++i)
            {
                ImGui::SameLineText("%02x", disassembleCtx.data.data[i]);
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(disassembleCtx.disassembled.c_str());
        }
    }
}

}  // namespace debugger::gui
