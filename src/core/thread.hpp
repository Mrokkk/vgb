#pragma once

#include <functional>

namespace core
{

using Task = std::move_only_function<void()>;

void async(Task task);
unsigned hardwareThreadCount();

}  // namespace core
