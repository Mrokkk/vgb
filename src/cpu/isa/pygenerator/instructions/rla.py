#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
uint16_t a = cpu.a;
uint16_t tmp = (a << 1) | cpu.f.c;
cpu.a = tmp;
cpu.f.c = tmp >> 8;
cpu.f.z = 0;
cpu.f.n = 0;
cpu.f.h = 0;
"""

def RLA(opcode : Opcode):
    return template

register_instruction_generator(RLA)
