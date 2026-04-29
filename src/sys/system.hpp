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

MaybeMappedFile mapFile(const char* pathname);

}  // namespace sys
