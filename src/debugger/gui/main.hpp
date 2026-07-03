#pragma once

#include "debugger/context.hpp"

namespace debugger::gui
{

void init(Context& ctx);
void deinit(Context& ctx);
void render(unsigned int gameTextureId);

}  // namespace debugger::gui
