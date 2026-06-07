#pragma once

#include <functional>

#include "fwd.hpp"
#include "utils/unique_ptr.hpp"

struct GameBoyInput
{
    bool a:1;
    bool b:1;
    bool start:1;
    bool select:1;
    bool up:1;
    bool down:1;
    bool right:1;
    bool left:1;
};

struct Input
{
    using GameBoyInputCallback = std::move_only_function<void(GameBoyInput)>;
    virtual ~Input() = default;
    virtual void update() = 0;
    virtual void subscribeForGameBoyInput(GameBoyInputCallback callback) = 0;
};

using InputPtr = utils::UniquePtr<Input>;

void createInput(GameBoy& gb, const Config& config);
