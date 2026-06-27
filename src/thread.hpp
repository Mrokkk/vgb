#pragma once

#include <functional>

using Task = std::move_only_function<void()>;

void async(Task task);
