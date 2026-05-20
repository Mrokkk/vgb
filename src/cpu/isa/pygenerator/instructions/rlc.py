#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const uint16_t a = {{ operand }};
const uint16_t tmp = (a << 1) | (a >> 7);
{{ store }};
cpu.f.c = tmp & 1;
cpu.f.z = not (tmp & 0xff);
cpu.f.n = 0;
cpu.f.h = 0;
"""

def RLC(opcode : Opcode):
    op = opcode.operands[0]
    operand = generate_load(op, opcode.width)
    store = generate_store(op, 'tmp', opcode.width)
    return render(template, locals())

register_instruction_generator(RLC)
