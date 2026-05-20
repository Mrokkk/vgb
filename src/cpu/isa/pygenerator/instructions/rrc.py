#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const uint8_t a = {{ operand }};
const uint8_t tmp = (a >> 1) | (a << 7);
{{ store }};
cpu.f.c = a & 1;
cpu.f.z = tmp == 0;
cpu.f.n = 0;
cpu.f.h = 0;
"""

def RRC(opcode : Opcode):
    op = opcode.operands[0]
    operand = generate_load(op, opcode.width)
    store = generate_store(op, 'tmp', opcode.width)
    return render(template, locals())

register_instruction_generator(RRC)
