#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ operand }};
int diff = cpu.a - operand;
cpu.f.n = 1;
cpu.f.z = not (diff & 0xff);
cpu.f.h = (cpu.a & 0xf) - (operand & 0xf) < 0;
cpu.f.c = diff < 0;
"""

def CP(opcode : Opcode):
    operand = generate_load(opcode.operands[1], opcode.width)
    return render(template, locals())

register_instruction_generator(CP)
