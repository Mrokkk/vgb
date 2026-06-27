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
cpu.sp -= 2;
cpu.mem.store16(cpu.sp, cpu.pc);
cpu.pushStackFrame();
cpu.pc = {{ addr }};
"""

def CALL(opcode : Opcode):
    if len(opcode.operands) == 2:
        condition = generate_load(opcode.operands[0], 1)
        addr = generate_load(opcode.operands[1], 16)
    else:
        addr = generate_load(opcode.operands[0], 16)

    return render(template, locals())

register_instruction_generator(CALL)
