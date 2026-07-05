#include "path.hpp"

#include "sys/platform.hpp"

namespace sys
{

DirIterator& DirIterator::operator++()
{
    auto res = platform.readDirectory(mPath, &mPlatformData);
    if (not res) [[unlikely]]
    {
        mPlatformData = nullptr;
    }
    mCurrentDirEntry = *res;
    return *this;
}

MaybeFileInfo Path::info() const
{
    return readFileInfo(mData.c_str());
}

bool Path::isFile() const
{
    auto i = info();
    return i and i->type == FileType::File;
}

bool Path::isDirectory() const
{
    auto i = info();
    return i and i->type == FileType::Directory;
}

DirIterator Path::readDirectory() const
{
    if (not platform.readDirectory) [[unlikely]]
    {
        return DirIterator(mData.c_str(), nullptr, {});
    }

    void* data = nullptr;

    auto res = platform.readDirectory(mData.c_str(), &data);

    if (not res) [[unlikely]]
    {
        return DirIterator(mData.c_str(), nullptr, {});
    }

    return DirIterator(mData.c_str(), data, *res);
}

DirEntries Path::readDirectoryList() const
{
    DirEntries list;
    for (const auto& e : readDirectory())
    {
        list.push_back(e);
    }
    return list;
}

}  // namespace sys
