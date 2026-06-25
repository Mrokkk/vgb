#pragma once

#include <functional>
#include <string>

namespace interpreter
{

using PrintOp = std::move_only_function<void(std::string line)>;

struct Operations
{
    PrintOp print;
};

}  // namespace interpreter
