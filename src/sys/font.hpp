#pragma once

#include <string>
#include <vector>

namespace sys
{

struct FontStyle final
{
    std::string path;
    std::string name;
};

struct Font final
{
    std::string family;
    std::vector<FontStyle> styles;
};

using Fonts = std::vector<Font>;

}  // namespace sys
