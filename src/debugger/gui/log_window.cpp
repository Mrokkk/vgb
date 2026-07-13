#include "log_window.hpp"

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_ext.h>

#include "core/logger.hpp"
#include "core/logger_reader.hpp"

namespace debugger::gui
{

static std::string buffer;
static std::vector<uint32_t> offsets;

static void onLog(Context& ctx, const core::LogEntry& entry)
{
    if (static_cast<int>(entry.severity) >= static_cast<int>(core::Severity::notice))
    {
        ctx.gui.messages.push_back(Message{
            .severity = entry.severity,
            .time = 0,
            .text = entry.message,
        });
    }

    char timeBuf[32];

    struct tm tmInfo;
    localtime_r(&entry.time, &tmInfo);

    strftime(timeBuf, sizeof(timeBuf), "%F %T", &tmInfo);

    offsets.push_back(buffer.size());

    if (entry.header)
    {
        buffer += fmt::format("[{}] [{}] {}: {}\n",
            timeBuf,
            entry.header,
            entry.location.func,
            entry.message);
    }
    else
    {
        buffer += fmt::format("[{}] {}: {}\n",
            timeBuf,
            entry.location.func,
            entry.message);
    }
}

void initLogWindow(Context& ctx)
{
    auto logCallback =
        [&ctx](const core::LogEntry& e)
        {
            onLog(ctx, e);
        };

    core::loggerReader.forEachLogEntry(logCallback);
    core::loggerReader.onLog(std::move(logCallback));
}

void drawLogWindow(Context& ctx)
{
    if (not ctx.gui.logWindow)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0, 0, 0, 0});

    if (auto window = ImGui::CreateWindow("Log", &ctx.gui.logWindow.get())) [[likely]]
    {
        ImGui::InputTextMultiline("##Log", buffer.data(), buffer.size() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

}  // namespace debugger::gui
