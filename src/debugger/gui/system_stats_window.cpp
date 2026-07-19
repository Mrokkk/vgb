#include "system_stats_window.hpp"

#include <imgui.h>
#include <imgui_ext.h>

#include "debugger/gui/helpers.hpp"
#include "sys/platform.hpp"
#include "utils/fixed_ring_buffer.hpp"
#include "utils/units.hpp"

namespace debugger::gui
{

static utils::FixedRingBuffer<float, 30> cpuUsagePlotData;
static utils::FixedRingBuffer<float, 30> allocUsagePlotData;

static float getCpuUsageAt(void*, int idx)
{
    return cpuUsagePlotData[idx];
}

static float getAllocUsageAt(void*, int idx)
{
    return allocUsagePlotData[idx];
}

void drawSystemStatsWindow(Context& ctx)
{
    CREATE_WINDOW("System stats", ctx.gui.systemStatsWindow);

    if (not sys::platform.getCpuUsage) [[unlikely]]
    {
        ImGui::TextDisabled("Unsupported by platform");
        return;
    }

    constexpr size_t updateInterval = 60;

    static double cpuUsage = 0;
    static size_t allocSize = 0;
    static const char* allocUnit;

    ONCE_PER_X_FRAMES(updateInterval)
    {
        cpuUsage = sys::platform.getCpuUsage();

        if (cpuUsagePlotData.size() == 0) [[unlikely]]
        {
            for (size_t i = 0; i < cpuUsagePlotData.capacity(); ++i)
            {
                cpuUsagePlotData.pushBack(0.f);
                allocUsagePlotData.pushBack(0.f);
            }
        }

        cpuUsagePlotData.pushBack(cpuUsage);

        const auto allocUsage = sys::platform.getAllocUsage();
        auto res = utils::humanReadable(allocUsage);
        allocSize = res.value;
        allocUnit = res.unit;

        allocUsagePlotData.pushBack(float(allocUsage) / MiB);
    }

    ImGui::Text("CPU usage: %0.1f%% (of single core)", cpuUsage);
    ImGui::PlotHistogram("##CPU usage", &getCpuUsageAt, nullptr, cpuUsagePlotData.size(), 0, nullptr, 0.f, 100.f, ImVec2{0, 80});
    ImGui::Text("Alloc usage: %zu %s", allocSize, allocUnit);
    ImGui::PlotHistogram("##Alloc usage", &getAllocUsageAt, nullptr, allocUsagePlotData.size(), 0, nullptr, 0.f, FLT_MAX, ImVec2{0, 80});
}

}  // namespace debugger::gui
