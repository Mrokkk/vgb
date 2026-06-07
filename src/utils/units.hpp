#pragma once

#include <cstddef>
#include <cstdint>

namespace utils
{

#define KiB 1024
#define MiB (KiB * KiB)
#define GiB (MiB * KiB)

namespace detail
{

struct HumanReadableSize
{
    const size_t      value;
    const char* const unit;
};

}  // namespace detail

detail::HumanReadableSize humanReadable(uint64_t value);

}  // namespace utils
