#pragma once

#include <cstddef>
#include <expected>
#include <string>

#include "utils/inline.hpp"
#include "utils/noncopyable.hpp"

namespace sys
{

struct MappedFile final : utils::NonCopyable
{
    MappedFile();
    MappedFile(bool readonly, void* ptr, size_t size);

    ~MappedFile();

    MappedFile(MappedFile&& other);
    MappedFile& operator=(MappedFile&& other);

    void reset();

    ALWAYS_INLINE void* getData() const
    {
        return mPtr;
    }

    template <typename T>
    ALWAYS_INLINE T* getData() const
    {
        return static_cast<T*>(mPtr);
    }

    ALWAYS_INLINE size_t getSize() const
    {
        return mSize;
    }

    ALWAYS_INLINE operator bool() const
    {
        return !!mPtr;
    }

private:
    bool   mReadonly;
    void*  mPtr;
    size_t mSize;
};

using MaybeMappedFile = std::expected<MappedFile, std::string>;

}  // namespace sys
