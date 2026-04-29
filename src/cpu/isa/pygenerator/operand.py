#!/usr/bin/env python3

from .cpu import cpu

class Operand:
    name     : str = None
    action   : str = None
    indirect : str = None
    value    : str = None
    width    : int = None

    def __repr__(self):
        if self.indirect == 'true':
            if self.action == 'Increment':
                return f'[{self.name}+]'
            elif self.action == 'Decrement':
                return f'[{self.name}-]'
            else:
                return f'[{self.name}]'
        if self.name == 'Builtin':
            return self.value
        elif self.action == 'Increment':
            return f'{self.name}+'
        elif self.action == 'Decrement':
            return f'{self.name}-'
        else:
            return f'{self.name}'

