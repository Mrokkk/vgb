#include "units.hpp"

#include <ostream>

namespace utils::detail
{

std::ostream& operator<<(std::ostream& os, const detail::HumanReadableSize::Data& d)
{
    if (d.value >= MiB)
    {
        os << d.value / MiB << " MiB";
    }
    else if (d.value >= KiB)
    {
        os << d.value / KiB << " KiB";
    }
    else
    {
        os << d.value << " B";
    }
    return os;
}

}  // namespace utils::detail
