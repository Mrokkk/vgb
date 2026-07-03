#pragma once

#include <cstdint>
#include <utility>

#include <imgui.h>

#include "utils/inline.hpp"

namespace ImGui
{

#define DEFINE_RAII_WRAPPER(NAME, BEGIN, END, ALWAYS) \
    struct NAME ## Wrapper \
    { \
        template <typename ...Args> \
        ALWAYS_INLINE constexpr NAME ## Wrapper(Args&&... args) \
        { \
            mIsVisible = BEGIN(std::forward<Args>(args)...); \
        } \
        ALWAYS_INLINE constexpr ~NAME ## Wrapper() \
        { \
            if (ALWAYS or mIsVisible) \
            { \
                END(); \
            } \
        } \
        ALWAYS_INLINE constexpr operator bool() const \
        { \
            return mIsVisible; \
        } \
    private: \
        bool mIsVisible; \
    }; \
    template <typename ...Args> \
    ALWAYS_INLINE constexpr auto Create##NAME(Args&&... args) \
    { \
        return NAME ## Wrapper(std::forward<Args>(args)...); \
    }

DEFINE_RAII_WRAPPER(Window, ImGui::Begin, ImGui::End, true)
DEFINE_RAII_WRAPPER(Child, ImGui::BeginChild, ImGui::EndChild, true)
DEFINE_RAII_WRAPPER(MainMenuBar, ImGui::BeginMainMenuBar, ImGui::EndMainMenuBar, false)
DEFINE_RAII_WRAPPER(Menu, ImGui::BeginMenu, ImGui::EndMenu, false)
DEFINE_RAII_WRAPPER(Table, ImGui::BeginTable, ImGui::EndTable, false)
DEFINE_RAII_WRAPPER(TreeNode, ImGui::TreeNode, ImGui::TreePop, false)
DEFINE_RAII_WRAPPER(PopupModal, ImGui::BeginPopupModal, ImGui::EndPopup, false)
DEFINE_RAII_WRAPPER(Combo, ImGui::BeginCombo, ImGui::EndCombo, false)
DEFINE_RAII_WRAPPER(TabBar, ImGui::BeginTabBar, ImGui::EndTabBar, false)
DEFINE_RAII_WRAPPER(TabItem, ImGui::BeginTabItem, ImGui::EndTabItem, false)

template <typename ...Args>
ALWAYS_INLINE bool SameLineButton(Args&&... args)
{
    ImGui::SameLine();
    return ImGui::Button(std::forward<Args>(args)...);
}

void SameLineText(const char* fmt, ...) IM_FMTARGS(1);

inline void SameLineText(const char* fmt, ...)
{
    ImGui::SameLine();
    va_list args;
    va_start(args, fmt);
    TextV(fmt, args);
    va_end(args);
}

inline bool WasWindowClicked(int button)
{
    return ImGui::IsWindowFocused() and ImGui::IsMouseReleased(button);
}

constexpr inline ImVec4 ColorFromHex(uint32_t color, float alpha = 1.0f)
{
    return ImVec4{
        ((color >> 16) & 0xff) / 255.0f,
        ((color >>  8) & 0xff) / 255.0f,
        ((color >>  0) & 0xff) / 255.0f,
        alpha,
    };
}

inline void BoolMenuItem(const char* name, const char* shortcut, bool& condition)
{
    if (ImGui::MenuItem(name, shortcut, condition))
    {
        condition ^= true;
    }
}

inline void BoolMenuItem(const char* name, const char* shortcut, bool* condition)
{
    if (ImGui::MenuItem(name, shortcut, *condition))
    {
        *condition ^= true;
    }
}

template <typename T, typename U = T>
void IntegerMenuItem(const char* name, const char* shortcut, T& variable, U value)
{
    if (ImGui::MenuItem(name, shortcut, variable == value))
    {
        variable = value;
    }
}

void ImageCentered(ImTextureID texture, size_t ratioX, size_t ratioY);

}  // namespace ImGui
