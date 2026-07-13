#!/usr/bin/env python3

mnemos_hpp_template = """\
#ifndef MNEMO
#define MNEMO(UPPER_CASE, LOWER_CASE)
#endif

#ifndef MNEMO_ILL
#define MNEMO_ILL(UPPER_CASE, LOWER_CASE)
#endif

{% for mnemo in mnemos %}
    {%- if mnemo == 'ILL' %}
MNEMO_ILL({{ mnemo }}, {{ mnemo.lower() }})
    {%- else %}
MNEMO({{ mnemo }}, {{ mnemo.lower() }})
    {%- endif %}
{%- endfor %}
"""
