#!/bin/env python3

import json
import os
import re
import shutil
import sys
import tempfile

def read_compile_commands():
    with open('compile_commands.json', 'r') as f:
        compile_commands = json.load(f)
    root = os.getcwd() + '/'

    mapping = {}

    for e in compile_commands:
        path : str = e['file']
        path = path.removeprefix(root)
        mapping[path] = e['command']

    return mapping


def format_command(tmpfile, original_command):
    regex = re.compile(r'-o [A-Za-z0-9_/\.]+ ')
    command = f'CCACHE_DISABLE=1 /usr/bin/time -v clang++ -ftime-trace {original_command.partition(' ')[2]} -Wno-error'
    command = re.sub(regex, f'-o {tmpfile} ', command)
    return command


def main():
    compile_commands = read_compile_commands()

    cpp = sys.argv[1]

    try:
        command : str = compile_commands[cpp]
    except:
        print(f'Invalid file: {cpp}')
        sys.exit(1)

    tmpfile = str(tempfile.mkstemp('.o')[1])

    command = format_command(tmpfile, command)

    os.system(command)

    tracing_src = f'{os.path.splitext(tmpfile)[0]}.json'
    tracing_dest = f'build/{os.path.splitext(cpp)[0]}.json'

    os.remove(tmpfile)

    output_dir = f'build/{os.path.dirname(cpp)}'

    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f'Failed to create dir: {e}')
        os.remove(tracing_src)
        sys.exit(1)

    try:
        shutil.move(tracing_src, tracing_dest)
    except Exception as e:
        print(f'Failed to move tracing file: {e}')
        sys.exit(1)

    print(f'Tracing available in: {os.path.abspath(tracing_dest)}')


if __name__ == '__main__':
    main()
