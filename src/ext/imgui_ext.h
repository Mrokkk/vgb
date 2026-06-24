#pragma once

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

}  // namespace ImGui
