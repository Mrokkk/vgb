#pragma once

#include <expected>
#include <string_view>

#include "assembler/assembler_fwd.hpp"

namespace assembler
{

struct Location final
{
    size_t           lineNo;
    size_t           pos;
    std::string_view fileName;
    std::string_view line;
};

using MaybeLocation = std::expected<Location, bool>;

void parse(const std::string_view& fileName, Context& mainContext);
MaybeLocation getCurrentLocation(Context* mainContext);
void setDebugging(bool enabled);

}  // namespace assembler
