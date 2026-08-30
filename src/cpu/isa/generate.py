#!/usr/bin/env python3

import jinja2
import json
import os
import sys

from pygenerator import *
from pygenerator.templates.instruction import *
from pygenerator.templates.instruction_set import *
from pygenerator.templates.mnemos import *

import pygenerator.instructions


def get_operand(operand, mnemo):
    op = Operand()

    name = operand['name']

    if 'decrement' in operand:
        op.action = 'Decrement'
    elif 'increment' in operand:
        op.action = 'Increment'
    else:
        op.action = None

    op.indirect = str(not operand['immediate']).lower()

    if name == 'n8':
        op.name = 'ImmU8'
        op.size = 1
        op.width = 1
    elif name == 'n16':
        op.name = 'Imm16'
        op.size = 2
        op.width = 2
    elif name == 'a8':
        op.name = 'Addr8'
        op.size = 1
        op.width = 2
    elif name == 'a16':
        op.name = 'Addr16'
        op.size = 2
        op.width = 2
    elif name == 'e8':
        op.name = 'ImmS8'
        op.size = 1
        op.width = 1
    elif name.isdigit():
        op.name = 'Builtin'
        op.size = 0
        op.value = hex(int(name))
        op.width = 1
    elif name[0] == '$':
        op.name = 'Builtin'
        op.size = 0
        op.value = hex(int(name[1:], 16))
        op.width = 1
    elif name in ('A', 'B', 'C', 'D', 'E', 'F', 'H', 'L'):
        if name == 'C' and mnemo in ('JP', 'JR', 'CALL', 'RET'): # Hack for JP/JR C, Addr16
            op.name = f'Flag{name}'
            op.size = 0
            op.width = 0
        else:
            op.name = name
            op.size = 0
            op.width = 1
    elif name in ('AF', 'BC', 'DE', 'HL', 'SP'):
        op.name = name
        op.size = 0
        op.width = 2
    elif name in ('Z', 'C', 'NZ', 'NC'):
        op.name = f'Flag{name}'
        op.size = 0
        op.width = 0
    else:
        raise Exception(f'Unknown operand: {name}')

    return op


def get_mnemonic(val):
    if 'ILLEGAL' in val:
        return 'ILL'
    if 'PREFIX' in val:
        return 'ILL'
    return val


def generate(instructions, mnemos, opcode_map, prefixed=False):
    opcodes = list()

    for k, v in instructions.items():
        opcode = Opcode()
        mnemo = get_mnemonic(v['mnemonic'])
        mnemos.add(mnemo)

        opcode.value = k.lower()
        opcode.prefix = '0xcb' if prefixed else '0x00'
        opcode.mnemo = mnemo
        opcode.size = v['bytes']
        opcode.cycles = max(v['cycles'])
        opcode.operands = list()

        if len(v['operands']) == 3: # Hack for LD
            opcode.operands.append(get_operand(v['operands'][0], opcode.mnemo))
            op2 = Operand()
            op2.name = 'SP_Plus_ImmS8'
            op2.size = 2
            op2.width = 16
            opcode.operands.append(op2)
        elif len(v['operands']) > 0:
            for operand in v['operands']:
                op = get_operand(operand, opcode.mnemo)
                opcode.operands.append(op)
            opcode.width = get_data_width(
                opcode.operands[0],
                opcode.operands[1] if len(opcode.operands) > 1 else None)

        opcodes.append(opcode)

        if not opcode.mnemo in opcode_map:
            opcode_map[opcode.mnemo] = []

        opcode_map[opcode.mnemo].append(opcode)

    return opcodes


def read_file(path):
    try:
        with open(path, 'r') as f:
            return f.read()
    except:
        return None


def write_file(path, content):
    file_content = read_file(path)

    if file_content == None:
        print(f'Creating {path}')
        with open(path, 'x') as f:
            f.write(content)
    elif file_content != content:
        print(f'Updating {path}')
        with open(path, 'w') as f:
            f.write(content)


def get_opcodes_json():
    with open('opcodes.json', 'r') as f:
        return json.load(f)


def main():
    mnemos = set()
    opcode_map = {}

    opcodes_json = get_opcodes_json()

    opcodes = generate(opcodes_json['unprefixed'], mnemos, opcode_map, prefixed=False)
    opcodes += generate(opcodes_json['cbprefixed'], mnemos, opcode_map, prefixed=True)

    assert len(opcodes) == 512

    instruction_cpps = {}

    declarations = []
    instructions = ['nullptr'] * 512

    for mnemo, ops in opcode_map.items():
        definitions = []
        for opcode in ops:
            generator = get_instruction_generator(opcode.mnemo)
            if not generator:
                continue
            instructions[int(opcode.value, 16) + (256 if opcode.prefix == '0xcb' else 0)] = instruction_function_name(opcode)
            instruction_code = generator(opcode).lstrip('\n')
            definitions.append(define_instruction(opcode, instruction_code))
            declarations.append(declare_instruction(opcode))

        instruction_cpps[mnemo] = jinja2.Template(instruction_cpp_template).render(
            instruction_defitions=definitions)

    isa_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
    instructions_dir = os.path.join(isa_dir, 'instructions')

    os.makedirs(instructions_dir, exist_ok=True)

    for mnemo, instruction_cpp in instruction_cpps.items():
        write_file(os.path.join(instructions_dir, f'{mnemo.lower()}.cpp'), instruction_cpp)

    mnemos_hpp = jinja2.Template(mnemos_hpp_template).render(mnemos=sorted(mnemos))

    write_file(os.path.join(isa_dir, 'mnemos.hpp'), mnemos_hpp)

    instruction_set_cpp = jinja2.Template(instruction_set_cpp_template).render(
        opcodes=opcodes,
        instructions=instructions,
        instruction_declarations=declarations)

    write_file(os.path.join(isa_dir, 'instruction_set.cpp'), instruction_set_cpp)


if __name__ == '__main__':
    main()
