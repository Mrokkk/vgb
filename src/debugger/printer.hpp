#pragma once

#include "cpu/sm83.hpp"
#include "debugger/state.hpp"

namespace debugger
{

void printInstruction(State& state, const cpu::SM83& cpu);
void printCpuRegs(State& state, const cpu::SM83& cpu);

}  // namespace debugger
