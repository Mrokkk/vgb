#!/bin/env python3

from pygenerator import *

def EI(opcode : Opcode):
    return 'cpu.ime = 1;'

register_instruction_generator(EI)
