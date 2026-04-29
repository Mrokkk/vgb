#!/usr/bin/env python3

instruction_cpp_template = """\
#include "cpu/sm83.hpp" // IWYU pragma: keep
#include "cpu/isa/instruction_set.hpp" // IWYU pragma: keep

namespace cpu::isa
{
{% for definition in instruction_defitions %}
{{ definition }}
{% endfor %}
}  // namespace cpu::isa
"""
