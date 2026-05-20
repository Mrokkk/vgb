#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template_hl = """
const auto op2 = {{ op2 }};
int diff = cpu.l + (op2 & 0xff);
cpu.f.n = 0;
cpu.f.c = diff >= 0x100;
cpu.l = diff;
diff = cpu.h + (op2 >> 8) + cpu.f.c;
cpu.f.h = (cpu.h & 0xf) + ((op2 >> 8) & 0xf) + cpu.f.c >= 0x10;
cpu.f.c = diff >= 0x100;
cpu.h = diff;
"""

template_sp = """
const auto op1 = {{ op1 }};
const auto op2 = {{ op2 }};
const int diff = op1 + op2;
cpu.f.n = 0;
cpu.f.h = (op1 & 0xf) + (op2 & 0xf) >= 0x10;
cpu.f.c = (op1 & 0xff) + (op2 & 0xff) >= 0x100;
{{ op1 }} = diff;
cpu.f.z = 0;
"""

template_8bit = """
const uint8_t op1 = {{ op1 }};
const uint8_t op2 = {{ op2 }};
const int diff = op1 + op2;
cpu.f.n = 0;
cpu.f.h = (op1 & 0xf) + (op2 & 0xf) >= 0x10;
cpu.f.c = diff >= 0x100;
{{ op1 }} = diff;
cpu.f.z = (diff & 0xff) == 0;
"""

def ADD(opcode : Opcode):
    op1 = generate_load(opcode.operands[0], opcode.width)
    op2 = generate_load(opcode.operands[1], opcode.width)
    if opcode.operands[0].name == 'HL':
        return render(template_hl, locals())
    elif opcode.operands[1].name == 'ImmS8':
        return render(template_sp, locals())
    else:
        return render(template_8bit, locals())

register_instruction_generator(ADD)
