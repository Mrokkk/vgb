#include "memory_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>
#include <imgui_memory_editor/imgui_memory_editor.h>

namespace debugger::gui
{

static ImU8 readMemory(const ImU8*, size_t addr, void*)
{
    return gb.cpu.mem.load8((uint16_t)addr);
}

static void writeMemory(ImU8*, size_t, ImU8, void*)
{
}

void initMemoryWindow(Context& ctx)
{
    ctx.gui.memEditor = new MemoryEditor;
    ctx.gui.memEditor->ReadFn          = &readMemory;
    ctx.gui.memEditor->WriteFn         = &writeMemory;
    ctx.gui.memEditor->Cols            = 8;
    ctx.gui.memEditor->OptShowOptions  = false;
    ctx.gui.memEditor->OptUpperCaseHex = false;
    ctx.gui.memEditor->Open            = false;
    ctx.gui.memEditorWindow            = &ctx.gui.memEditor->Open;

    core::IniSerializer::registerData("memEditorWindow", ctx.gui.memEditor->Open);
}

void deinitMemoryWindow(Context& ctx)
{
    if (ctx.gui.memEditorWindow)
    {
        delete ctx.gui.memEditor;
    }
}

void drawMemoryWindow(Context& ctx)
{
    auto memEditor = ctx.gui.memEditor;

    if (not memEditor->Open)
    {
        return;
    }

    constexpr auto addressSpace = memory::Map::ADDRESS_SPACE_SIZE;

    MemoryEditor::Sizes s;
    memEditor->CalcSizes(s, addressSpace, 0);
    ImGui::SetNextWindowSize(ImVec2(s.WindowWidth, s.WindowWidth * 0.60f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));

    auto window = ImGui::CreateWindow("Memory", &memEditor->Open, ImGuiWindowFlags_NoScrollbar);

    if (not window) [[unlikely]]
    {
        return;
    }

    if (ImGui::InputText("Go to address", ctx.gui.addrBuffer, sizeof(ctx.gui.addrBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        char* tmp;
        auto addr = strtoul(ctx.gui.addrBuffer, &tmp, 16);
        memEditor->GotoAddr = addr;
    }

    ImGui::Text("Go to: ");

    if (ImGui::SameLineButton("PC"))
    {
        memEditor->GotoAddr = gb.cpu.pc;
    }

    if (ImGui::SameLineButton("SP"))
    {
        memEditor->GotoAddr = gb.cpu.sp;
    }

    if (ImGui::SameLineButton("VRAM"))
    {
        memEditor->GotoAddr = memory::Map::VRAM.start;
    }

    if (ImGui::SameLineButton("IO"))
    {
        memEditor->GotoAddr = memory::Map::IO.start;
    }

    memEditor->DrawContents(nullptr, addressSpace, 0);

    if (memEditor->ContentsWidthChanged)
    {
        memEditor->CalcSizes(s, addressSpace, 0);
        ImGui::SetWindowSize(ImVec2(s.WindowWidth, ImGui::GetWindowSize().y));
    }
}

}  // namespace debugger::gui
