#include "platform.hpp"

#include "error.hpp"
#include "sys/mapped_file.hpp"
#include "sys/supervision.hpp"

namespace sys
{

Platform platform;

MaybeMappedFile mapFile(const char* pathname, bool readOnly)
{
    if (not platform.mapFileImpl) [[unlikely]]
    {
        return error("Platform does not support file mapping");
    }

    auto result = platform.mapFileImpl(pathname, readOnly);

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

void frame()
{
    pingSupervision();
    platform.input->update();
    platform.renderer->render();
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

MaybeError writeToFile(const char* pathname, const void* data, size_t size)
{
    if (not platform.writeToFile) [[unlikely]]
    {
        return error("writeToFile not implemented");
    }
    return platform.writeToFile(pathname, data, size);
}

std::string getDefaultConfigDir()
{
    if (not platform.getDefaultConfigDir) [[unlikely]]
    {
        return "";
    }
    return platform.getDefaultConfigDir();
}

MaybeFileInfo readFileInfo(const char* pathname)
{
    if (not platform.readFileInfo) [[unlikely]]
    {
        return error("readFileInfo not implemented");
    }
    return platform.readFileInfo(pathname);
}

bool isFile(const char* pathname)
{
    auto info = readFileInfo(pathname);
    return info and info->type == FileType::File;
}

bool isDirectory(const char* pathname)
{
    auto info = readFileInfo(pathname);
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

MaybeError createDirectory(const char* pathname)
{
    if (not platform.createDirectory) [[unlikely]]
    {
        return error("createDirectory not implemented");
    }
    return platform.createDirectory(pathname);
}

}  // namespace sys
