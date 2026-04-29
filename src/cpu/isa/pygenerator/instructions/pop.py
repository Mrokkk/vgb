#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
{{ operand }} = cpu.mem.load16(cpu.sp);
cpu.sp += 2;
"""

def POP(opcode : Opcode):
    operand = operand_to_code(opcode.operands[0], opcode.width)
    return render(template, locals())

register_instruction_generator(POP)
