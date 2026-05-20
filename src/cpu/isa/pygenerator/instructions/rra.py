#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
const uint8_t a = cpu.a;
const uint8_t tmp = (a >> 1) | (cpu.f.c << 7);
cpu.a = tmp;
cpu.f.c = a & 1;
cpu.f.z = 0;
cpu.f.n = 0;
cpu.f.h = 0;
"""

def RRA(opcode : Opcode):
    return template

register_instruction_generator(RRA)
