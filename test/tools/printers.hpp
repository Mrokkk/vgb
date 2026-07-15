#pragma once

// IWYU pragma: always_keep

#include <string>

#include "test/tools/test_framework.hpp"
#include "assembler/assembler.hpp"
#include "src/cpu/exception.hpp"
#include "src/cpu/register.hpp"

template <>
struct TestStringConverter<assembler::MaybeRom>
{
    static std::string convert(const assembler::MaybeRom& value);
};

template <>
struct TestStringConverter<cpu::Register16>
{
    static std::string convert(cpu::Register16 value);
};

template <>
struct TestStringConverter<cpu::Register8>
{
    static std::string convert(cpu::Register8 value);
};

template <>
struct TestStringConverter<cpu::Exception>
{
    static std::string convert(cpu::Exception value);
};
