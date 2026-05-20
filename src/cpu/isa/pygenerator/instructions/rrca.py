#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
int tmp = cpu.a & 1;
cpu.a = (cpu.a >> 1) | (tmp << 7);
cpu.f.z = 0;
cpu.f.h = 0;
cpu.f.n = 0;
cpu.f.c = tmp;
"""

def RRCA(opcode : Opcode):
    return template

register_instruction_generator(RRCA)
