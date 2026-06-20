#pragma once

#include <cstddef>
#include <expected>
#include <string>

#include "fwd.hpp"
#include "utils/inline.hpp"
#include "utils/noncopyable.hpp"

namespace sys
{

void initialize(const Config& config);
void stacktraceLog();
void pingSupervision();
void stopSupervision();

using MaybeString = std::expected<std::string, std::string>;

MaybeString readLineFromStdin(const std::string_view& prompt);

struct MappedFile final : utils::NonCopyable
{
    MappedFile()
        : mReadonly(false)
        , mPtr(nullptr)
        , mSize(0)
    {
    }

    MappedFile(bool readonly, void* ptr, size_t size);
    ~MappedFile();

    MappedFile(MappedFile&& other);
    MappedFile& operator=(MappedFile&& other);

    ALWAYS_INLINE void* getData() const
    {
        return mPtr;
    }

    template <typename T>
    ALWAYS_INLINE T* getData() const
    {
        return reinterpret_cast<T*>(mPtr);
    }

    ALWAYS_INLINE size_t getSize() const
    {
        return mSize;
    }

private:
    bool   mReadonly;
    void*  mPtr;
    size_t mSize;
};

using MaybeMappedFile = std::expected<MappedFile, std::string>;

bool doesDirExist(const char* pathname);
bool doesFileExist(const char* pathname);

MaybeMappedFile mapFile(const char* pathname, bool readOnly = true);
std::expected<bool, std::string> saveToFile(const char* pathname, const void* data, size_t size);

}  // namespace sys
