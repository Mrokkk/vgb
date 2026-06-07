#include <cstdint>

#include <imgui.h>
#include <imgui_ext.h>

#include "cpu/sm83.hpp"
#include "debugger/games/game.hpp"
#include "debugger/games/registry.hpp"
#include "game_boy.hpp"

namespace debugger::games
{

struct PokemonRed final : Game
{
    void drawUi() override;
};

REGISTER_GAME(PokemonRed, "POKEMON RED");

struct Options
{
    uint8_t textSpeed:4;
    uint8_t unused:2;
    uint8_t battleStyle:1;
    uint8_t battleAnimation:1;
};

static const char* readString(char* buffer, const uint8_t* data, size_t size)
{
    constexpr char charMap[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ():;[]abcdefghijklmnopqrstuvwxyzedlstv"
        "                                "
        "\'PM-rm?!.   >>vM$*./,F0123456789";

    size_t i;
    for (i = 0; i < size; ++i)
    {
        auto byte = data[i];
        if (byte == 0x50)
        {
            break;
        }
        buffer[i] = byte < 0x80
            ? ' '
            : charMap[byte - 0x80];
    }
    buffer[i] = 0;
    return buffer;
}

#define RADIO_BUTTON(NAME, VARIABLE, VALUE) \
    do if (ImGui::RadioButton(NAME, VARIABLE == VALUE)) \
    { \
        VARIABLE = VALUE; \
    } \
    while (0)

#define RADIO_BUTTONS_1(TITLE, ...) \
    do \
    { \
        ImGui::Text(TITLE); \
        ImGui::SameLine(); \
        RADIO_BUTTON(__VA_ARGS__); \
    } \
    while (0)

#define RADIO_BUTTONS_2(TITLE, NAME0, VARIABLE0, VALUE0, ...) \
    do \
    { \
        RADIO_BUTTONS_1(TITLE, NAME0, VARIABLE0, VALUE0); \
        ImGui::SameLine(); \
        RADIO_BUTTON(__VA_ARGS__); \
    } \
    while (0)

#define RADIO_BUTTONS_3(TITLE, NAME0, VARIABLE0, VALUE0, NAME1, VARIABLE1, VALUE1, ...) \
    do \
    { \
        ImGui::Text(TITLE); \
        RADIO_BUTTONS_2(TITLE, NAME0, VARIABLE0, VALUE0, NAME1, VARIABLE1, VALUE1); \
        ImGui::SameLine(); \
        RADIO_BUTTON(__VA_ARGS__); \
    } \
    while (0)

void PokemonRed::drawUi()
{
    auto wram = gb.cpu.mem.bankedWorkRam.data;
    auto& options = *reinterpret_cast<Options*>(&wram[0x355]);

    ImGui::SeparatorText("Pokemon Red");

    if (auto _ = ImGui::CreateTreeNode("Options"))
    {
        RADIO_BUTTONS_2("Battle animation",
            "on", options.battleAnimation, 0,
            "off", options.battleAnimation, 1);

        RADIO_BUTTONS_2("Battle style",
            "set", options.battleStyle, 1,
            "shift", options.battleStyle, 0);

        RADIO_BUTTONS_3("Battle style",
            "fast", options.textSpeed, 1,
            "medium", options.textSpeed, 3,
            "slow", options.textSpeed, 5);
    }

    if (auto _ = ImGui::CreateTreeNode("Player"))
    {
        char name[12];
        ImGui::Text("Name: %s", readString(name, wram + 0x158, 10));

        const auto pokemons = wram[0x163];

        if (auto _ = ImGui::CreateTreeNode("##Pokemons", "Party: %u Pokemons", pokemons))
        {
            for (uint8_t i = 0; i < pokemons; ++i)
            {
                ImGui::PushID(i);
                ImGui::Text("Pokemon ID: %u", wram[0x16b + i * 44]);

                char name[32];
                ImGui::Text("Name: %s", readString(name, wram + 0x2b5 + i * 11, 10));
                ImGui::PopID();
            }
        }
    }
}

}  // namespace debugger::games
