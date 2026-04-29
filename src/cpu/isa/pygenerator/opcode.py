#!/usr/bin/env python3

from .operand import Operand

class Opcode:
    value    : str           = None
    prefix   : str           = None
    mnemo    : str           = None
    operands : list[Operand] = None
    size     : int           = None
    width    : int           = None
    cycles   : list[int]     = None
    write    : str           = None

    def __repr__(self):
        return f'{self.mnemo} {', '.join([str(op) for op in self.operands])}'.strip()
