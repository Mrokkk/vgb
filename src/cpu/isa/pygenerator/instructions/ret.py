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
cpu.pc = cpu.mem.load16(cpu.sp);
cpu.sp += 2;
cpu.popStackFrame();
"""

def RET(opcode : Opcode):
    if len(opcode.operands) == 1:
        condition = generate_load(opcode.operands[0], 1)

    return render(template, locals())

register_instruction_generator(RET)
