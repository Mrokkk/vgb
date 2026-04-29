#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
uint{{ opcode.width }}_t value = {{ operand }};
value = (value >> 4) | (value << 4);
{{ store }};
cpu.f.n = 0;
cpu.f.h = 0;
cpu.f.c = 0;
cpu.f.z = not value;
"""

def SWAP(opcode : Opcode):
    operand = generate_load(opcode.operands[0], opcode.width)
    store = generate_store(opcode.operands[0], 'value', opcode.width)
    return render(template, locals())

register_instruction_generator(SWAP)
