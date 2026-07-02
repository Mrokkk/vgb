#pragma once

#include <cstddef>
#include <vector>

#include "utils/noncopyable.hpp"
#include "utils/ring_buffer_impl.hpp"

namespace utils
{

template <typename T>
struct RingBuffer final
    : public detail::RingBufferImpl<std::vector<T>>
    , NonCopyable
{
    using U = std::vector<T>;

    constexpr RingBuffer()
    {
    }

    constexpr RingBuffer(size_t size)
        : detail::RingBufferImpl<U>(size)
    {
    }
};

}  // namespace utils
