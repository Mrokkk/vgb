#!/usr/bin/env python3

class Types:
    uint8_t = 'uint8_t'
    int8_t = 'int8_t'

def negate(value):
    return f'not {value}'

def cast(ctype, value):
    return f'({ctype}){value}'

def statement(statement):
    return f'{statement};'

def assignment(lhs, rhs):
    return f'{lhs} = {rhs}'

def addition(lhs, rhs):
    return f'{lhs} + {rhs}'

def call(name, *args):
    return f'{name}({', '.join([arg for arg in args])})'
