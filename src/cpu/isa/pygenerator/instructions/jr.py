#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
{%- if condition %}
if (not ({{ condition }}))
{
    return;
}
{%- endif %}
const int8_t addr = {{ addr }};
if (cpu.pc + addr == cpu.pc - {{ opcode.size }}) [[unlikely]]
{
    cpu.exc.reportInfiniteLoop();
    return;
}
cpu.pc = cpu.pc + addr;
"""

def JR(opcode : Opcode):
    if len(opcode.operands) == 2:
        condition = generate_load(opcode.operands[0], 1)
        addr = generate_load(opcode.operands[1], 16)
    else:
        addr = generate_load(opcode.operands[0], 16)

    return render(template, locals())

register_instruction_generator(JR)
