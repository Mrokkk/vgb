#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const uint16_t operand = {{ operand }};
const uint16_t tmp = (operand << 1) | cpu.f.c;
{{ store }};
cpu.f.c = tmp >> 8;
cpu.f.z = not (tmp & 0xff);
cpu.f.n = 0;
cpu.f.h = 0;
"""

def RL(opcode : Opcode):
    operand = generate_load(opcode.operands[0], opcode.width)
    store = generate_store(opcode.operands[0], 'tmp', opcode.width)
    return render(template, locals())

register_instruction_generator(RL)
