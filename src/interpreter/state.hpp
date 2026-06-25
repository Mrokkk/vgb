#pragma once

#include <string>

#include "interpreter/operations.hpp"

namespace interpreter
{

struct State final
{
    std::string currentCommand;
    void*       defaultUserData;
    Operations  ops;
};

}  // namespace interpreter
