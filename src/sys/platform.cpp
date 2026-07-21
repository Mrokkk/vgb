#include "platform.hpp"

#include "error.hpp"
#include "sys/mapped_file.hpp"

namespace sys
{

Platform platform;

#define READ_PATH(SRC, DEST) \
    char DEST[1024]; \
    if (SRC.size() + 1 >= sizeof(DEST)) [[unlikely]] \
    { \
        return error("Filename too long"); \
    } \
    memcpy(DEST, SRC.data(), SRC.size()); \
    DEST[SRC.size()] = '\0'

MaybeMappedFile mapFile(const std::string_view& path, bool readOnly)
{
    if (not platform.mapFileImpl) [[unlikely]]
    {
        return error("Platform does not support file mapping");
    }

    READ_PATH(path, buffer);

    auto result = platform.mapFileImpl(buffer, readOnly);

    if (not result) [[unlikely]]
    {
        return error(std::move(result.error()));
    }

    return MappedFile{
        readOnly,
        result->ptr,
        result->size
    };
}

void abortMainThread()
{
    if (platform.abortMainThread) [[likely]]
    {
        platform.abortMainThread();
    }
    else
    {
        std::abort();
    }
}

void stacktraceLog()
{
    if (platform.stacktraceLog) [[likely]]
    {
        platform.stacktraceLog();
    }
}

MaybeError writeToFile(const std::string_view& path, const void* data, size_t size)
{
    if (not platform.writeToFile) [[unlikely]]
    {
        return error("writeToFile not implemented");
    }
    READ_PATH(path, buffer);
    return platform.writeToFile(buffer, data, size);
}

std::string getDefaultConfigDir()
{
    if (not platform.getDefaultConfigDir) [[unlikely]]
    {
        return "";
    }
    return platform.getDefaultConfigDir();
}

MaybeFileInfo readFileInfo(const std::string_view& path)
{
    if (not platform.readFileInfo) [[unlikely]]
    {
        return error("readFileInfo not implemented");
    }
    READ_PATH(path, buffer);
    return platform.readFileInfo(buffer);
}

bool isFile(const std::string_view& path)
{
    auto info = readFileInfo(path);
    return info and info->type == FileType::File;
}

bool isDirectory(const std::string_view& path)
{
    auto info = readFileInfo(path);
    return info and info->type == FileType::Directory;
}

Fonts getFonts()
{
    if (not platform.getFonts) [[unlikely]]
    {
        return {};
    }
    return platform.getFonts();
}

MaybeError createDirectory(const std::string_view& path)
{
    if (not platform.createDirectory) [[unlikely]]
    {
        return error("createDirectory not implemented");
    }
    READ_PATH(path, buffer);
    return platform.createDirectory(buffer);
}

}  // namespace sys
