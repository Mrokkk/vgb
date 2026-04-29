#!/bin/env python3

from .cpp import *

class cpu:
    a   = 'cpu.a'
    b   = 'cpu.b'
    c   = 'cpu.c'
    d   = 'cpu.d'
    e   = 'cpu.e'
    f   = 'cpu.f'
    h   = 'cpu.h'
    l   = 'cpu.l'
    af  = 'cpu.af'
    bc  = 'cpu.bc'
    de  = 'cpu.de'
    hl  = 'cpu.hl'
    sp  = 'cpu.sp'
    pc  = 'cpu.pc'

    class f:
        c = 'cpu.f.c'
        h = 'cpu.f.h'
        n = 'cpu.f.n'
        z = 'cpu.f.z'

    class mem:
        def load8(value):
            return call('cpu.mem.load8', value)

        def load16(value):
            return call('cpu.mem.load16', value)

        def load(width, value):
            return call(f'cpu.mem.load{width}', value)

        def store8(addr, value):
            return call(f'cpu.mem.store8', addr, value)

        def store16(addr, value):
            return call('cpu.mem.store16', addr, value)

        def store(width, addr, value):
            return call(f'cpu.mem.store{width}', addr, value)
