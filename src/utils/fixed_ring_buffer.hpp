#pragma once

#include "ring_buffer_impl.hpp"
#include "utils/immobile.hpp"

namespace utils
{

template <typename T, size_t Size>
struct FixedRingBuffer : detail::RingBufferImpl<std::array<T, Size>>, utils::Immobile
{
    constexpr FixedRingBuffer() = default;
};

}  // namespace utils
