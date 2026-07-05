#pragma once

#include <string_view>

namespace test::tools
{

struct PlatformFake
{
    PlatformFake();
    ~PlatformFake();

    PlatformFake& addFile(std::string_view path);
    PlatformFake& addFile(std::string_view path, void* data, size_t size);
    PlatformFake& addDirectory(std::string_view path);
    PlatformFake& setWorkingDirectory(std::string_view path);
};

}  // namespace test::tools
