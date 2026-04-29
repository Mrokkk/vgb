#!/bin/env python3

from pygenerator.isa import *
from pygenerator.opcode import *

def NOP(opcode : Opcode):
    return '(void)0;'

register_instruction_generator(NOP)
