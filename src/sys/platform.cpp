#include "platform.hpp"

#include "sys/mapped_file.hpp"
#include "sys/supervision.hpp"

namespace sys
{

Platform platform;

MaybeMappedFile mapFile(const char* pathname, bool readOnly)
{
    if (not platform.mapFileImpl) [[unlikely]]
    {
        return std::unexpected("Platform does not support file mapping");
    }

    auto result = platform.mapFileImpl(pathname, readOnly);

    if (not result) [[unlikely]]
    {
        return std::unexpected(std::move(result.error()));
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

void abort()
{
    if (platform.abort) [[likely]]
    {
        platform.abort();
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
        return std::unexpected("writeToFile operation not supported");
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

bool doesDirExist(const char* pathname)
{
    if (not platform.doesDirExist) [[unlikely]]
    {
        return false;
    }
    return platform.doesDirExist(pathname);
}

bool doesFileExist(const char* pathname)
{
    if (not platform.doesFileExist) [[unlikely]]
    {
        return false;
    }
    return platform.doesFileExist(pathname);
}

Fonts getFonts()
{
    if (not platform.getFonts) [[unlikely]]
    {
        return {};
    }
    return platform.getFonts();
}

}  // namespace sys
