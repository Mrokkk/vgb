#include "mapped_file.hpp"

#include "sys/platform.hpp"

namespace sys
{

MappedFile::MappedFile()
    : mReadonly(false)
    , mPtr(nullptr)
    , mSize(0)
{
}

MappedFile::MappedFile(bool readonly, void* ptr, size_t size)
    : mReadonly(readonly)
    , mPtr(ptr)
    , mSize(size)
{
}

MappedFile::~MappedFile()
{
    if (platform.unmapFileImpl)
    {
        auto res = platform.unmapFileImpl(mPtr, mSize);
        (void)res;
    }
}

MappedFile::MappedFile(MappedFile&& other)
    : mReadonly(other.mReadonly)
    , mPtr(other.mPtr)
    , mSize(other.mSize)
{
    other.mPtr = nullptr;
    other.mSize = 0;
}

MappedFile& MappedFile::operator=(MappedFile&& other)
{
    mReadonly = other.mReadonly;
    mPtr = other.mPtr;
    mSize = other.mSize;
    other.mPtr = nullptr;
    other.mSize = 0;
    return *this;
}

}  // namespace sys
