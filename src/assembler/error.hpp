#pragma once

#include <string>
#include <vector>

#include "assembler/lexer.hpp"

namespace assembler
{

struct Error final
{
    std::string   message;
    MaybeLocation location;
};

using Errors = std::vector<Error>;

}  // namespace assembler
