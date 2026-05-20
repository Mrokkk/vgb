#!/bin/env python3

from pygenerator.cpu import *
from pygenerator.isa import *
from pygenerator.opcode import *

template = """
if (cpu.f.n)
{
    if (cpu.f.h)
    {
        cpu.a += 0xfa;
    }
    if (cpu.f.c)
    {
        cpu.a += 0xa0;
    }
}
else
{
    int a = cpu.a;
    if ((cpu.a & 0xf) > 0x9 or cpu.f.h)
    {
        a += 0x6;
    }
    if ((a & 0x1f0) > 0x90 or cpu.f.c)
    {
        a += 0x60;
        cpu.f.c = 1;
    }
    else
    {
        cpu.f.c = 0;
    }
    cpu.a = a;
}
cpu.f.h = 0;
cpu.f.z = not cpu.a;
"""

def DAA(opcode : Opcode):
    return template

register_instruction_generator(DAA)
