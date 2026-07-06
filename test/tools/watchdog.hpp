#pragma once

#include "src/utils/function_ref.hpp"

namespace test::tools
{

void createWatchdog();
void deleteWatchdog();
void withWatchdog(double timeout, utils::FunctionRef<void()> task);

}  // namespace test::tools
