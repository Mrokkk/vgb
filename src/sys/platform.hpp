#pragma once

#include <expected>

#include "sys/font.hpp"
#include "sys/input.hpp"
#include "sys/mapped_file.hpp"
#include "sys/renderer.hpp"

namespace sys
{

using MaybeError = std::expected<bool, std::string>;

struct Mapped final
{
    void*  ptr;
    size_t size;
};

using MaybeMapped = std::expected<Mapped, std::string>;

struct Platform
{
    void        (*abort)();
    void        (*stacktraceLog)();
    MaybeError  (*writeToFile)(const char* pathname, const void* data, size_t size);
    std::string (*getDefaultConfigDir)();
    bool        (*doesDirExist)(const char* pathname);
    bool        (*doesFileExist)(const char* pathname);
    Fonts       (*getFonts)();
    MaybeMapped (*mapFileImpl)(const char* pathname, bool readOnly);
    MaybeError  (*unmapFileImpl)(void* ptr, size_t size);
    double      (*getCpuUsage)();
    size_t      (*getAllocUsage)();

    RendererPtr renderer;
    InputPtr    input;
};

extern Platform platform;

void frame();
void abort();
MaybeMappedFile mapFile(const char* pathname, bool readOnly = true);
void stacktraceLog();
MaybeError writeToFile(const char* pathname, const void* data, size_t size);
std::string getDefaultConfigDir();
bool doesDirExist(const char* pathname);
bool doesFileExist(const char* pathname);
Fonts getFonts();

}  // namespace sys
