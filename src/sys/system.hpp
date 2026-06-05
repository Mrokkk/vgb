#pragma once

#include <expected>
#include <string>

namespace sys
{

void initialize();
void finalize();
void stacktraceLog();
void pingSupervision();
void stopSupervision();

using MaybeString = std::expected<std::string, std::string>;

MaybeString readLineFromStdin(const std::string_view& prompt);

struct MappedFile
{
    void*  ptr;
    size_t size;
};

using MaybeMappedFile = std::expected<MappedFile, std::string>;

bool doesFileExist(const char* pathname);
MaybeMappedFile mapFile(const char* pathname, bool readOnly = true);
std::expected<bool, std::string> saveToFile(const char* pathname, const void* data, size_t size);

}  // namespace sys
