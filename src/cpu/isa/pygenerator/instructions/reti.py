#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
cpu.pc = cpu.mem.load16(cpu.sp);
cpu.sp += 2;
cpu.ime = 1;
"""

def RETI(opcode : Opcode):
    return template

register_instruction_generator(RETI)
