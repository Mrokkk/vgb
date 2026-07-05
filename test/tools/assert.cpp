#include "assert.hpp"

#include <cstdarg>
#include <cstdio>
#include <stdexcept>

namespace test::tools
{

[[noreturn]] void throwImpl(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    throw std::runtime_error(buffer);
}

}  // namespace test::tools
