#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ operand }};
const int diff = {{ dest }} - operand - cpu.f.c;
cpu.f.n = 1;
cpu.f.h = ({{ dest }} & 0xf) - (operand & 0xf) - cpu.f.c < 0;
cpu.f.c = diff < 0;
{{ dest }} = diff;
cpu.f.z = not {{ dest }};
"""

def SBC(opcode : Opcode):
    dest = operand_to_code(opcode.operands[0], 8)
    operand = generate_load(opcode.operands[1], 8)
    return render(template, locals())

register_instruction_generator(SBC)
