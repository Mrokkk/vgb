#pragma once

// IWYU pragma: always_keep

#include <doctest.h>

#include "assembler/assembler.hpp"
#include "src/cpu/exception.hpp"
#include "src/cpu/register.hpp"

namespace doctest
{

template <>
struct StringMaker<assembler::MaybeRom>
{
    static String convert(const assembler::MaybeRom& value);
};

template <>
struct StringMaker<cpu::Register16>
{
    static String convert(cpu::Register16 value);
};

template <>
struct StringMaker<cpu::Register8>
{
    static String convert(cpu::Register8 value);
};

template <>
struct StringMaker<cpu::Exception>
{
    static String convert(cpu::Exception value);
};

}  // namespace doctest
