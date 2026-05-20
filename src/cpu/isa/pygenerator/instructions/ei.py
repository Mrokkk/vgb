#!/bin/env python3

from pygenerator import *

def EI(opcode : Opcode):
    return 'cpu.scheduleEi();'

register_instruction_generator(EI)
