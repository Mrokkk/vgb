#pragma once

#include "cpu/sm83.hpp"

namespace debugger
{

void printInstruction(const cpu::SM83& cpu);
void printCpuRegs(const cpu::SM83& cpu);

}  // namespace debugger
