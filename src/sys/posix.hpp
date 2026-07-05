#pragma once

#include "fwd.hpp"

namespace sys::posix
{

#define SYS_PATH_SEPARATOR   '/'
#define SYS_PATH_CURRENT_DIR "."

void initialize(const Config& config);

}  // namespace sys::posix
