#include "main.hpp"

#include <imgui.h>
#include <imgui_ext.h>
#include <imgui_internal.h>

#include "core/logger.hpp"
#include "core/logger_reader.hpp"
#include "debugger/context.hpp"
#include "debugger/gui/callstack_window.hpp"
#include "debugger/gui/cartridge_window.hpp"
#include "debugger/gui/config_window.hpp"
#include "debugger/gui/console_window.hpp"
#include "debugger/gui/cpu_window.hpp"
#include "debugger/gui/disassembly_window.hpp"
#include "debugger/gui/emulation_window.hpp"
#include "debugger/gui/game_window.hpp"
#include "debugger/gui/helpers.hpp"
#include "debugger/gui/io_window.hpp"
#include "debugger/gui/lcd_window.hpp"
#include "debugger/gui/log_window.hpp"
#include "debugger/gui/map_window.hpp"
#include "debugger/gui/memory_window.hpp"
#include "debugger/gui/messages.hpp"
#include "debugger/gui/system_stats_window.hpp"
#include "sys/platform.hpp"
#include "utils/inline.hpp"

namespace debugger::gui
{

static void* openIni(ImGuiContext*, ImGuiSettingsHandler* s, const char*)
{
    return s->UserData;
}

static void readIniLine(ImGuiContext*, ImGuiSettingsHandler*, void*, const char* line)
{
    auto result = core::IniSerializer::deserializeLine(line);
    if (not result)
    {
        core::logger.error().write("INI error: {}", result.error());
    }
}

static void writeIni(ImGuiContext*, ImGuiSettingsHandler* s, ImGuiTextBuffer* buf)
{
    buf->reserve(200);
    buf->appendf("[%s][Data]\n", s->TypeName);
    auto str = core::IniSerializer::serialize();
    buf->append(str.c_str(), str.c_str() + str.size());
}

void init(Context& ctx)
{
    ctx.gui.messageTime = 180;
    ctx.gui.messageFadeOutTime = 60;
    ctx.gui.fonts = sys::getFonts();
    ctx.gui.fontSize = 16;

    initLogWindow(ctx);
    initMemoryWindow(ctx);

    {
        auto iniPath= sys::getDefaultConfigDir();
        iniPath += "/gui.ini";
        ctx.gui.iniPath = std::move(iniPath);
    }

    {
        ImGuiSettingsHandler iniHandler;
        iniHandler.TypeName = "GUI";
        iniHandler.TypeHash = ImHashStr("GUI");
        iniHandler.ReadOpenFn = &openIni;
        iniHandler.ReadLineFn = &readIniLine;
        iniHandler.WriteAllFn = &writeIni;
        iniHandler.UserData = &ctx.gui;
        ImGui::AddSettingsHandler(&iniHandler);
    }

    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::LoadIniSettingsFromDisk(ctx.gui.iniPath.c_str());

    ctx.gui.defaultFont = io.FontDefault;

    if (ctx.gui.fontFamily.get().empty())
    {
        ctx.gui.fontFamily = "(default)";
        ctx.gui.fontStyle = "(default)";
        ctx.gui.currentFont = nullptr;
        ctx.gui.currentFontStyle = nullptr;
    }
    else
    {
        for (const auto& font : ctx.gui.fonts)
        {
            if (font.family == ctx.gui.fontFamily.get())
            {
                for (const auto& style : font.styles)
                {
                    if (style.name == ctx.gui.fontStyle.get())
                    {
                        io.FontDefault = io.Fonts->AddFontFromFileTTF(style.path.c_str());
                        ctx.gui.currentFont = &font;
                        ctx.gui.currentFontStyle = &style;
                    }
                }
                if (not ctx.gui.currentFontStyle)
                {
                    ctx.gui.currentFontStyle = &font.styles[0];
                }
            }
        }
    }

    auto& style = ImGui::GetStyle();
    style.FontSizeBase      = ctx.gui.fontSize;
    style.WindowBorderSize  = 0;
    style.FrameRounding     = 5.0f;
    style.ScrollbarRounding = 5.0f;
}

void deinit(Context& ctx)
{
    ImGui::SaveIniSettingsToDisk(ctx.gui.iniPath.c_str());
    core::loggerReader.onLog(nullptr);
    deinitMemoryWindow(ctx);
}

#define SPEED_MENU_ITEM(SPEED) \
    do if (ImGui::MenuItem(#SPEED "x", nullptr, gb.speedMultiplier == SPEED)) \
    { \
        gb.speedMultiplier = SPEED; \
    } \
    while (0)

ALWAYS_INLINE static void drawMenuBar(Context& ctx)
{
    auto mainMenu = ImGui::CreateMainMenuBar();

    if (not mainMenu) [[unlikely]]
    {
        return;
    }

    if (auto menu = ImGui::CreateMenu("File"))
    {
        if (ImGui::MenuItem("Configuration"))
        {
            ctx.gui.configWindow = true;
        }
        if (ImGui::MenuItem("Quit"))
        {
            gb.cpu.exc.reportUserInterruption();
        }
    }
    if (auto menu = ImGui::CreateMenu("Emulation"))
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            if (ImGui::MenuItem("Start"))
            {
                gb.start();
            }
        }
        else
        {
            if (ImGui::MenuItem("Stop"))
            {
                gb.stop();
            }
        }

        if (auto menu = ImGui::CreateMenu("Speed"))
        {
            SPEED_MENU_ITEM(1);
            SPEED_MENU_ITEM(2);
            SPEED_MENU_ITEM(3);
            SPEED_MENU_ITEM(4);
            SPEED_MENU_ITEM(5);
        }
    }

    if (auto menu = ImGui::CreateMenu("View"))
    {
        ImGui::BoolMenuItem("Emulation", nullptr, ctx.gui.emulationWindow);
        ImGui::BoolMenuItem("Cartridge", nullptr, ctx.gui.cartridgeWindow);
        ImGui::BoolMenuItem("CPU", nullptr, ctx.gui.cpuWindow);
        ImGui::BoolMenuItem("Memory", nullptr, ctx.gui.memEditorWindow);
        ImGui::BoolMenuItem("Disassembly window", nullptr, ctx.gui.disassemblyWindow);
        ImGui::BoolMenuItem("Callstack window", nullptr, ctx.gui.callstackWindow);
        ImGui::BoolMenuItem("IO", nullptr, ctx.gui.ioWindow);
        ImGui::BoolMenuItem("Map", nullptr, ctx.gui.mapWindow);
        ImGui::BoolMenuItem("Console", "`", ctx.gui.consoleWindow);
        ImGui::BoolMenuItem("Game", nullptr, ctx.gui.gameWindow);
        ImGui::BoolMenuItem("Log", nullptr, ctx.gui.logWindow);
        ImGui::BoolMenuItem("System stats", nullptr, ctx.gui.systemStatsWindow);
        ImGui::BoolMenuItem("Style Editor", nullptr, ctx.gui.styleEditorWindow);
        ImGui::BoolMenuItem("Demo", nullptr, ctx.gui.demoWindow);
    }
}

ALWAYS_INLINE static void drawStyleEditorWindow(Context& ctx)
{
    CREATE_WINDOW("Style Editor", ctx.gui.styleEditorWindow);
    ImGui::ShowStyleEditor();
}

ALWAYS_INLINE static void drawDemoWindow(Context& ctx)
{
    if (ctx.gui.demoWindow)
    {
        ImGui::ShowDemoWindow(&ctx.gui.demoWindow.get());
    }
}

void render(unsigned int gameTextureId)
{
    const auto dockspaceId = ImGui::GetID("vgb dockspace");
    const auto viewport = ImGui::GetMainViewport();

    ImGui::DockSpaceOverViewport(dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    auto& ctx = *reinterpret_cast<Context*>(gb.debuggerData);

    drawMenuBar(ctx);
    drawLcdWindow(ctx, gameTextureId);
    drawCartridgeWindow(ctx);
    drawCpuWindow(ctx);
    drawMemoryWindow(ctx);
    drawIoWindow(ctx);
    drawConsoleWindow(ctx);
    drawMapWindow(ctx);
    drawGameWindow(ctx);
    drawLogWindow(ctx);
    drawEmulationWindow(ctx);
    drawDisassemblyWindow(ctx);
    drawCallstackWindow(ctx);
    drawSystemStatsWindow(ctx);

    drawStyleEditorWindow(ctx);
    drawDemoWindow(ctx);

    drawMessages(ctx);
    drawConfigWindow(ctx);

    if (ImGui::IsKeyReleased(ImGuiKey_GraveAccent))
    {
        if (ctx.gui.consoleWindow ^= true)
        {
            ImGui::SetWindowFocus("Console");
            ctx.gui.focusCmdLine = true;
        }
    }

    ctx.gui.frameCounter++;
}

}  // namespace debugger::gui
