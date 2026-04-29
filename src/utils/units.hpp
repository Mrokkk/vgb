#pragma once

#include <cstdint>
#include <iosfwd>

namespace utils
{

#define KiB 1024
#define MiB (KiB * KiB)
#define GiB (MiB * KiB)

namespace detail
{

struct HumanReadableSize
{
    struct Data { uint32_t value; };

    Data operator()(uint32_t value) const
    {
        return Data{value};
    }
};

std::ostream& operator<<(std::ostream& os, const HumanReadableSize::Data& d);

}  // namespace detail

static constexpr detail::HumanReadableSize humanReadableSize;

}  // namespace utils
