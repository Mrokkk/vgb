#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
cpu.a = ~cpu.a;
cpu.f.n = 1;
cpu.f.h = 1;
"""

def CPL(opcode : Opcode):
    return template

register_instruction_generator(CPL)
