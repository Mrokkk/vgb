#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ operand }};
const int diff = {{ dest }} + operand;
cpu.f.n = 0;
cpu.f.h = ({{ dest }} & 0xf) + (operand & 0xf) >= 0x10;
cpu.f.c = diff >= 0x100;
{{ dest }} = diff;
cpu.f.z = not {{ dest }};
"""

# FIXME: 16bit version do not modify Z flag

def ADD(opcode : Opcode):
    dest = operand_to_code(opcode.operands[0], opcode.width)
    operand = generate_load(opcode.operands[1], opcode.width)
    return render(template, locals())

register_instruction_generator(ADD)
