#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

def HALT(opcode : Opcode):
    return 'cpu.state = SM83::State::Halted;'

register_instruction_generator(HALT)
