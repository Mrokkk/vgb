#!/usr/bin/env python3

instruction_set_cpp_template = """\
#include "instruction_set.hpp"

#include "cpu/isa/opcode.hpp"

namespace cpu::isa
{
{% for instruction in instruction_declarations %}
{{ instruction }}
{%- endfor %}

InstructionSet::InstructionSet()
    : mOpcodes{
{%- for opcode in opcodes %}
        /* {{ opcode.prefix }} {{ opcode.value }} */ Opcode{ // {{ opcode }}
            .mnemo   = Opcode::{{ opcode.mnemo }},
            .opCount = {{ opcode.operands.__len__() }},
            .value   = {{ opcode.value }},
    {%- if opcode.operands.__len__() > 0 %}
            .op      = {
        {%- for operand in opcode.operands %}
                Operand{
                    .type     = Operand::{{ operand.name }},
            {%- if operand.action %}
                    .action   = Operand::Action::{{ operand.action }},
            {%- endif %}
            {%- if operand.indirect == "true" %}
                    .indirect = {{ operand.indirect }},
            {%- endif %}
            {%- if operand.value %}
                    .value    = {{ operand.value }},
            {%- endif %}
                },
        {%- endfor %}
            },
    {%- endif %}
            .bytes   = {{ opcode.size }},
            .cycles  = {{ opcode.cycles }},
        },
{%- endfor %}
    }
    , mInstructions{
{%- for instruction in instructions %}
        {{ instruction }},
{%- endfor %}
    }
{
}

}  // namespace cpu::isa
"""
