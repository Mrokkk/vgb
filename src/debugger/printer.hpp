#pragma once

#include "debugger/context.hpp"

namespace debugger
{

void printInstruction(Context& ctx);
void printCpuRegs(Context& ctx);

}  // namespace debugger
