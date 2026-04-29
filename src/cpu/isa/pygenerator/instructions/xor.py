#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const uint{{ opcode.width }}_t diff = {{ dest }} ^ {{ src }};
{{ dest }} = diff;
cpu.f.z = not diff;
cpu.f.n = 0;
cpu.f.h = 0;
cpu.f.c = 0;
"""

def XOR(opcode : Opcode):
    dest = operand_to_code(opcode.operands[0], opcode.width)
    src  = generate_load(opcode.operands[1], opcode.width)
    return render(template, locals())

register_instruction_generator(XOR)
