#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

def LD(opcode : Opcode):
    dest = opcode.operands[0]
    src  = opcode.operands[1]

    return statement(
        generate_store(
            dest,
            generate_load(src, opcode.width),
            opcode.width))

register_instruction_generator(LD)
