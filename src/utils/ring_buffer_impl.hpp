#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "utils/function_ref.hpp"
#include "utils/inline.hpp"

namespace utils::detail
{

template <typename T>
struct RingBufferImpl
{
    using value_type = typename T::value_type;
protected:
    ALWAYS_INLINE constexpr RingBufferImpl()
        : mStart(0)
        , mSize(0)
        , mCurrent(0)
    {
    }

    template <typename U = T>
    requires (std::is_constructible_v<U, size_t>)
    ALWAYS_INLINE constexpr RingBufferImpl(size_t size)
        : mStart(0)
        , mSize(0)
        , mCurrent(0)
        , mBuffer(size)
    {
    }

public:
    ALWAYS_INLINE constexpr void pushFront(value_type value)
    {
        const auto maxSize = mBuffer.size();

        if (static_cast<int>(--mStart) < 0)
        {
            mStart = maxSize - 1;
        }

        if (mSize < maxSize)
        {
            mSize++;
        }
        else if (static_cast<int>(--mCurrent) < 0)
        {
            mCurrent = maxSize - 1;
        }

        mBuffer[mStart] = std::move(value);
    }

    ALWAYS_INLINE constexpr void pushBack(value_type value)
    {
        const auto maxSize = mBuffer.size();

        if (mCurrent >= maxSize)
        {
            mCurrent = 0;
        }

        mBuffer[mCurrent++] = std::move(value);

        if (mSize == maxSize)
        {
            if (++mStart >= maxSize)
            {
                mStart = 0;
            }
        }
        else
        {
            ++mSize;
        }
    }

    constexpr const value_type& at(size_t i) const
    {
        i += mStart;
        if (i >= mBuffer.size())
        {
            i -= mBuffer.size();
        }
        return mBuffer[i];
    }

    ALWAYS_INLINE constexpr const value_type& operator[](size_t i) const
    {
        return at(i);
    }

    constexpr void clear()
    {
        mCurrent = mSize = mStart = 0;
    }

    constexpr size_t capacity() const
    {
        return mBuffer.size();
    }

    constexpr size_t size() const
    {
        return mSize;
    }

    constexpr void forEach(const FunctionRef<void(const value_type&)> callback) const
    {
        if (mSize == 0) [[unlikely]]
        {
            return;
        }
        if (mCurrent <= mStart)
        {
            for (size_t i = mStart; i < mBuffer.size(); ++i)
            {
                callback(mBuffer[i]);
            }
            for (size_t i = 0; i < mCurrent; ++i)
            {
                callback(mBuffer[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < mCurrent; ++i)
            {
                callback(mBuffer[i]);
            }
        }
    }

protected:
    size_t         mStart;
    size_t         mSize;
    size_t         mCurrent;
    T              mBuffer;
};

}  // namespace utils::detail
