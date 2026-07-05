#pragma once

#include <ctime>
#include <expected>

#include "sys/font.hpp"
#include "sys/input.hpp"
#include "sys/mapped_file.hpp"
#include "sys/renderer.hpp"

#ifdef __unix__
#include "posix.hpp"
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
    MaybeError    (*writeToFile)(const char* pathname, const void* data, size_t size);
    std::string   (*getDefaultConfigDir)();
    MaybeFileInfo (*readFileInfo)(const char* pathname);
    Fonts         (*getFonts)();
    MaybeMapped   (*mapFileImpl)(const char* pathname, bool readOnly);
    MaybeError    (*unmapFileImpl)(void* ptr, size_t size);
    double        (*getCpuUsage)();
    size_t        (*getAllocUsage)();
    MaybeDirEntry (*readDirectory)(const char* pathname, void** ptr);
    MaybeError    (*createDirectory)(const char* pathname);

    RendererPtr renderer;
    InputPtr    input;
};

extern Platform platform;

void frame();
void abortMainThread();
MaybeMappedFile mapFile(const char* pathname, bool readOnly = true);
void stacktraceLog();
MaybeError writeToFile(const char* pathname, const void* data, size_t size);
std::string getDefaultConfigDir();
MaybeFileInfo readFileInfo(const char* pathname);
bool isFile(const char* pathname);
bool isDirectory(const char* pathname);
Fonts getFonts();
MaybeError createDirectory(const char* pathname);

}  // namespace sys
