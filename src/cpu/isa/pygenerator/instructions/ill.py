#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

def ILL(opcode : Opcode):
    return f'cpu.exc.reportInvalidOpcode({opcode.value});'

register_instruction_generator(ILL)
