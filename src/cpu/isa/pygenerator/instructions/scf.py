#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
cpu.f.n = 0;
cpu.f.h = 0;
cpu.f.c = 1;
"""

def SCF(opcode : Opcode):
    return template

register_instruction_generator(SCF)
