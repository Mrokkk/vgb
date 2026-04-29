#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
uint{{ opcode.width }}_t operand = {{ operand }};
operand &= ~(1 << {{ bit }});
{{ store }};
"""

def RES(opcode : Opcode):
    bit = operand_to_code(opcode.operands[0], opcode.width)
    operand = generate_load(opcode.operands[1], opcode.width)
    store = generate_store(opcode.operands[1], 'operand', opcode.width)
    return render(template, locals())

register_instruction_generator(RES)
