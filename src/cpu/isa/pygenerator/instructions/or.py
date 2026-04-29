#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ operand }};
cpu.a |= operand;
cpu.f.z = not cpu.a;
cpu.f.n = 0;
cpu.f.h = 0;
cpu.f.c = 0;
"""

def OR(opcode : Opcode):
    operand = generate_load(opcode.operands[1], opcode.width)
    return render(template, locals())

register_instruction_generator(OR)
