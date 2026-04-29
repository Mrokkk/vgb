#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const auto operand = {{ src }};
const int diff = operand - 1;
{{ store }};
{%- if opcode.width == 8 %}
cpu.f.h = (operand & 0xf) == 0x0;
cpu.f.n = 1;
cpu.f.z = not (diff & 0xff);
{%- endif %}
"""

def DEC(opcode : Opcode):
    op = opcode.operands[0]
    src = generate_load(op, opcode.width)
    store = generate_store(op, 'diff', opcode.width)
    return render(template, locals())

register_instruction_generator(DEC)
