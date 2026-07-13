#pragma once

#include <ctime>
#include <expected>

#include "sys/font.hpp"
#include "sys/input.hpp"
#include "sys/mapped_file.hpp"
#include "sys/renderer.hpp"

#ifdef __unix__
#include "posix.hpp" // IWYU pragma: keep
#else
#endif

namespace sys
{

using MaybeError = std::expected<bool, std::string>;

struct Mapped final
{
    void*  ptr;
    size_t size;
};

using MaybeMapped = std::expected<Mapped, std::string>;

enum class FileType
{
    File,
    Directory,
    Other,
};

struct FileInfo final
{
    FileType type;
    size_t   size;
    clock_t  modificationTime;
};

using MaybeFileInfo = std::expected<FileInfo, std::string>;

struct DirEntry final
{
    const char* name;
    FileInfo    info;
};

using MaybeDirEntry = std::expected<DirEntry, std::string>;

struct Platform
{
    void          (*abortMainThread)();
    void          (*stacktraceLog)();
    MaybeError    (*writeToFile)(const char* path, const void* data, size_t size);
    std::string   (*getDefaultConfigDir)();
    MaybeFileInfo (*readFileInfo)(const char* path);
    Fonts         (*getFonts)();
    MaybeMapped   (*mapFileImpl)(const char* path, bool readOnly);
    MaybeError    (*unmapFileImpl)(void* ptr, size_t size);
    double        (*getCpuUsage)();
    size_t        (*getAllocUsage)();
    MaybeDirEntry (*readDirectory)(const char* path, void** ptr);
    MaybeError    (*createDirectory)(const char* path);

    RendererPtr renderer;
    InputPtr    input;
};

extern Platform platform;

void frame();
void abortMainThread();
MaybeMappedFile mapFile(const std::string_view& path, bool readOnly = true);
void stacktraceLog();
MaybeError writeToFile(const std::string_view& path, const void* data, size_t size);
std::string getDefaultConfigDir();
MaybeFileInfo readFileInfo(const std::string_view& path);
bool isFile(const std::string_view& path);
bool isDirectory(const std::string_view& path);
Fonts getFonts();
MaybeError createDirectory(const std::string_view& path);

}  // namespace sys
