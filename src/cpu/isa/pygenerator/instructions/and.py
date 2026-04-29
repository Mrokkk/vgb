#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ src }};
{{ dest }} &= operand;
cpu.f.z = not cpu.a;
cpu.f.n = 0;
cpu.f.h = 1;
cpu.f.c = 0;
"""

def AND(opcode : Opcode):
    src = generate_load(opcode.operands[1], opcode.width)
    dest = operand_to_code(opcode.operands[0], opcode.width)
    return render(template, locals())

register_instruction_generator(AND)
