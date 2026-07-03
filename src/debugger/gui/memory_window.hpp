#pragma once

#include "debugger/context.hpp"

namespace debugger::gui
{

void initMemoryWindow(Context& ctx);
void deinitMemoryWindow(Context& ctx);
void drawMemoryWindow(Context& ctx);

}  // namespace debugger::gui
