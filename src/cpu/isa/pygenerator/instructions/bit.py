#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ operand }};
cpu.f.z = not (operand & (1 << {{ bit }}));
cpu.f.n = 0;
cpu.f.h = 1;
"""

def BIT(opcode : Opcode):
    bit = operand_to_code(opcode.operands[0], opcode.width)
    operand = generate_load(opcode.operands[1], opcode.width)
    return render(template, locals())

register_instruction_generator(BIT)
