#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
if (cpu.pc - {{ opcode.size }} == {{ addr }}) [[unlikely]]
{
    cpu.exc.reportInfiniteLoop();
    return;
}
cpu.sp -= 2;
cpu.mem.store16(cpu.sp, cpu.pc);
cpu.pc = {{ addr }};
"""

def RST(opcode : Opcode):
    addr = generate_load(opcode.operands[0], 8)
    return render(template, locals())

register_instruction_generator(RST)
