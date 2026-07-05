#pragma once

#include <string>
#include <utility>

#include "sys/platform.hpp"

namespace sys
{

struct Path;

struct DirIterator final
{
    constexpr DirIterator begin() const
    {
        return *this;
    }

    constexpr DirIterator end() const
    {
        return DirIterator(mPath, nullptr, {});
    }

    constexpr bool operator==(const DirIterator& other) const
    {
        return mPlatformData == other.mPlatformData;
    }

    constexpr operator bool() const
    {
        return mPlatformData;
    }

    const DirEntry& operator*() const
    {
        return mCurrentDirEntry;
    }

    DirIterator& operator++();

private:
    constexpr DirIterator(const char* path, void* platformData, DirEntry dirEntry)
        : mPath(path)
        , mPlatformData(platformData)
        , mCurrentDirEntry(dirEntry)
    {
    }

    friend Path;
    const char* mPath;
    void*       mPlatformData;
    DirEntry    mCurrentDirEntry;
};

using DirEntries = std::vector<DirEntry>;

struct Path final
{
    constexpr static inline char Separator = SYS_PATH_SEPARATOR;
    constexpr static inline auto CurrentDir = SYS_PATH_CURRENT_DIR;

    constexpr Path() = default;

    constexpr Path(std::string data) // TODO: remove duplicated separators
        : mData(std::move(data))
    {
    }

    constexpr Path& operator/=(std::string_view sv)
    {
        stripSeparators(sv);
        if (not sv.empty())
        {
            mData += Separator;
            mData += sv;
        }
        return *this;
    }

    constexpr void replaceExtension(std::string_view ext)
    {
        auto pos = mData.find_last_of('.');
        if (pos == mData.npos)
        {
            pos = mData.size();
        }
        mData.replace(pos, mData.size() - pos, ext);
    }

    constexpr std::string_view extension() const
    {
        auto pos = mData.find_last_of('.');
        if (pos == mData.npos) [[unlikely]]
        {
            return std::string_view(mData.data() + mData.size(), 0);
        }
        return std::string_view(mData.data() + pos, mData.size() - pos);
    }

    constexpr std::string_view dirname() const
    {
        auto pos = mData.find_last_of(Separator);
        if (pos == mData.npos)
        {
            return std::string_view(CurrentDir);
        }
        else if (pos == 0)
        {
            pos++;
        }
        return std::string_view(mData.data(), mData.data() + pos);
    }

    constexpr std::string_view basename() const
    {
        auto pos = mData.find_last_of(Separator);
        if (pos == mData.npos or (pos == 0 and mData.size() == 1))
        {
            return std::string_view(mData);
        }
        return std::string_view(mData.data() + pos + 1, mData.size() - pos - 1);
    }

    constexpr const std::string& string() const
    {
        return mData;
    }

    constexpr const char* cString() const
    {
        return mData.c_str();
    }

    constexpr std::string&& release()
    {
        return std::move(mData);
    }

    constexpr operator bool() const
    {
        return not mData.empty();
    }

    MaybeFileInfo info() const;
    bool isFile() const;
    bool isDirectory() const;
    DirIterator readDirectory() const;
    DirEntries readDirectoryList() const;

private:
    constexpr void stripSeparators(std::string_view& sv) const
    {
        while (sv.size() and sv[0] == Separator)
        {
            sv.remove_prefix(1);
        }
        while (sv.size() and sv[sv.size() - 1] == Separator)
        {
            sv.remove_suffix(1);
        }
    }

    std::string mData;
};

}  // namespace sys
