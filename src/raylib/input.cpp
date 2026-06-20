#include "input.hpp"

#include <cstdint>

#include <raylib.h>

#include "game_boy.hpp"
#include "../input.hpp"
#include "utils/unique_ptr.hpp"

namespace raylib
{

struct RaylibInput final : Input
{
    RaylibInput();
    void update() override;
    void subscribeForGameBoyInput(GameBoyInputCallback callback) override;

    GameBoyInput gbInput;
    GameBoyInputCallback gbInputCallback;
};

RaylibInput::RaylibInput()
{
    SetExitKey(KEY_NULL);
}

bool operator==(GameBoyInput lhs, GameBoyInput rhs)
{
    return *reinterpret_cast<const uint8_t*>(&lhs)
        == *reinterpret_cast<const uint8_t*>(&rhs);
}

void RaylibInput::update()
{
    GameBoyInput tmp;
    if (not gb.inputEnabled) [[unlikely]]
    {
        tmp = {};
    }
    else
    {
        tmp.a = IsKeyDown(KEY_X);
        tmp.b = IsKeyDown(KEY_Z);
        tmp.start = IsKeyDown(KEY_ENTER);
        tmp.select = IsKeyDown(KEY_BACKSPACE);
        tmp.up = IsKeyDown(KEY_UP);
        tmp.down = IsKeyDown(KEY_DOWN);
        tmp.left = IsKeyDown(KEY_LEFT);
        tmp.right = IsKeyDown(KEY_RIGHT);
    }
    if (tmp != gbInput)
    {
        gbInput = tmp;
        if (gbInputCallback) [[likely]]
        {
            gbInputCallback(gbInput);
        }
    }

    if (IsKeyReleased(KEY_TAB))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            gb.speedMultiplier -= 1;
            if (gb.speedMultiplier == 0)
            {
                gb.speedMultiplier = 1;
            }
        }
        else
        {
            gb.speedMultiplier += 1;
        }
    }
}

void RaylibInput::subscribeForGameBoyInput(GameBoyInputCallback callback)
{
    gbInputCallback = std::move(callback);
}

void createInput(GameBoy& gb, const Config&)
{
    gb.input = utils::makeUnique<RaylibInput>();
}

}  // namespace raylib
