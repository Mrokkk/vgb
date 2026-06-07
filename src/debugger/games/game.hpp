#pragma once

#include "utils/unique_ptr.hpp"

namespace debugger::games
{

struct Game
{
    virtual ~Game() = default;
    virtual void drawUi() = 0;
};

using GamePtr = utils::UniquePtr<Game>;

}  // namespace debugger::games
