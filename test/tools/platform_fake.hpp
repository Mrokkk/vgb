#pragma once

#include <string_view>

namespace test::tools
{

struct PlatformFake
{
    PlatformFake();
    ~PlatformFake();

    PlatformFake& addFile(const std::string_view& path);
    PlatformFake& addFile(const std::string_view& path, void* data, size_t size);
    PlatformFake& addFile(const std::string_view& path, const std::string_view& sv);
    PlatformFake& addDirectory(const std::string_view& path);
    PlatformFake& setWorkingDirectory(const std::string_view& path);
};

}  // namespace test::tools
