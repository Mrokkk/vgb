#include "units.hpp"

namespace utils
{

detail::HumanReadableSize humanReadable(uint64_t value)
{
    if (value >= MiB)
    {
        return {value / MiB, "MiB"};
    }
    else if (value >= KiB)
    {
        return {value / KiB, "KiB"};
    }
    else
    {
        return {value, "B"};
    }
}

}  // namespace utils
