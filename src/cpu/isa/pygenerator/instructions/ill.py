#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

def ILL(opcode : Opcode):
    return 'cpu.exc = Exception::InvalidOpcode;'

register_instruction_generator(ILL)
