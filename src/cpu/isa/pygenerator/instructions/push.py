#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
cpu.sp -= 2;
cpu.mem.store16(cpu.sp, {{ operand }});
"""

def PUSH(opcode : Opcode):
    operand = operand_to_code(opcode.operands[0], opcode.width)
    return render(template, locals())

register_instruction_generator(PUSH)
