#!/usr/bin/env python3

import jinja2

from .cpu import *

instruction_generators = {}


class Macros:
    declare_instruction = 'DECLARE_INSTRUCTION'
    instruction_name = 'INSTRUCTION_NAME'
    define_instruction = 'DEFINE_INSTRUCTION'


def get_instruction_generator(mnemo):
    global instruction_generators
    if mnemo in instruction_generators:
        return instruction_generators[mnemo]
    else:
        return None


def register_instruction_generator(function):
    global instruction_generators
    mnemo = function.__name__
    instruction_generators[mnemo] = function


def instruction_function_name(opcode):
    return call(Macros.instruction_name, opcode.mnemo, opcode.value)


def declare_instruction(opcode):
    return statement(call(Macros.declare_instruction, opcode.mnemo, opcode.value))


def remove_trailing_whitespace(string : str):
    return '\n'.join([line.rstrip(' ') for line in string.splitlines()])


def define_instruction(opcode, code):
    tmp = '\n{\n'
    for line in code.splitlines():
        tmp += f'    {line}\n'
    tmp += '}'
    code = tmp

    return remove_trailing_whitespace(f'{Macros.define_instruction}({opcode.mnemo}, {opcode.value}, // {opcode}, {code})')


def get_data_width(op1, op2=None):
    if not op2:
        return op1.width * 8 if op1.indirect == 'false' else 8
    return op1.width * 8 if op1.indirect == 'false' else op2.width * 8


def operand_to_code(op, data_width):
    if op.name in ('A', 'B', 'C', 'D', 'E', 'F', 'H', 'L', 'AF', 'BC', 'DE', 'HL', 'SP'):
        inner = cpu.__dict__[op.name.lower()]
        if op.action == 'Increment':
            inner += '++'
        elif op.action == 'Decrement':
            inner += '--'
    elif op.name == 'Addr8':
        inner = addition('0xff00', 'imm.load8()')
    elif op.name in ('Addr16', 'Imm16'):
        inner = 'imm.load16()'
    elif op.name == 'ImmU8':
        inner = 'imm.load8()'
    elif op.name == 'ImmS8':
        inner = cast(Types.int8_t, 'imm.load8()')
    elif op.name == 'SP_Plus_ImmS8':
        inner = addition(cpu.sp, cast(Types.int8_t, 'imm.load8()'))
    elif op.name in 'FlagC':
        inner = cpu.f.c
    elif op.name in 'FlagZ':
        inner = cpu.f.z
    elif op.name in 'FlagNC':
        inner = negate(cpu.f.c)
    elif op.name in 'FlagNZ':
        inner = negate(cpu.f.z)
    elif op.name == 'Builtin':
        inner = op.value
    else:
        raise Exception(f'Incorrect operand: {op}')

    return inner

def generate_store(dest, src_code, data_width):
    inner = operand_to_code(dest, data_width)

    if dest.indirect == 'true':
        return cpu.mem.store(data_width, inner, src_code)
    else:
        return assignment(inner, src_code)


def generate_load(src, data_width):
    inner = operand_to_code(src, data_width)

    if src.indirect == 'true':
        return cpu.mem.load(data_width, inner)
    else:
        return inner


def render(template, *variables):
    return jinja2.Template(template).render(*variables)
