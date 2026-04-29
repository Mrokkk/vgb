#!/bin/env python3

from pygenerator import *

def DI(opcode : Opcode):
    return 'cpu.ime = 0;'

register_instruction_generator(DI)
