#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template_sp_imms8 = """
const uint16_t op1 = cpu.sp;
const auto op2 = (int8_t)imm.load8();
cpu.hl = op1 + op2;
cpu.f.z = 0;
cpu.f.n = 0;
cpu.f.h = (op1 & 0xf) + (op2 & 0xf) >= 0x10;
cpu.f.c = (op1 & 0xff) + (op2 & 0xff) >= 0x100;
"""

def LD(opcode : Opcode):
    dest = opcode.operands[0]
    src  = opcode.operands[1]

    if src.name == 'SP_Plus_ImmS8':
        return render(template_sp_imms8, locals())

    return statement(
        generate_store(
            dest,
            generate_load(src, opcode.width),
            opcode.width))

register_instruction_generator(LD)
