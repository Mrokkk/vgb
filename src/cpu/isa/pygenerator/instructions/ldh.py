#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

def LDH(opcode : Opcode):
    dest = opcode.operands[0]
    src  = opcode.operands[1]

    data_width = opcode.width

    # LDH [C], A
    if dest.name == 'C':
        return f'cpu.mem.store{data_width}(0xff00 + cpu.c, {generate_load(src, data_width)});'
    # LDH A, [C]
    elif src.name == 'C':
        return f'{operand_to_code(dest, data_width)} = cpu.mem.load{data_width}(0xff00 + cpu.c);'

    # LDH [Addr8], A
    # LDH A, [Addr8]
    return statement(
        generate_store(
            dest,
            generate_load(src, data_width),
            data_width))

register_instruction_generator(LDH)
