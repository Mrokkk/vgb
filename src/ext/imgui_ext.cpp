#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_ext.h"

namespace ImGui
{

static ImVec2 ScaleToRatio(const ImVec2& vec, int ratioX, int ratioY)
{
    ImVec2 res = vec;
    if (vec.x * ratioY > vec.y * ratioX)
    {
        res.x = vec.y * ratioX / ratioY;
    }
    else
    {
        res.y = vec.x * ratioY / ratioX;
    }
    return res;
}

void ImageCentered(ImTextureID texture, size_t ratioX, size_t ratioY)
{
    const auto viewportSize = ImGui::GetContentRegionAvail();
    const auto imageSize = ImGui::ScaleToRatio(viewportSize, ratioX, ratioY);
    ImGui::SetCursorPos((viewportSize - imageSize) * 0.5 + ImGui::GetCursorPos());
    ImGui::Image(texture, imageSize);
}

bool Button(const char* label, bool enabled, const ImVec2& size)
{
    if (not enabled)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    auto result = Button(label, size);
    if (not enabled)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    return result;
}

}  // namespace ImGui
