#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "assembler/error.hpp"
#include "utils/immobile.hpp"

namespace assembler
{

enum class SectionType
{
    ROM0,
};

struct Section final
{
    SectionType type;
    size_t      currentOffset;
};

struct UserSection final
{
    const Section* section;
    size_t         currentOffset;
};

using UserSections = std::map<std::string_view, UserSection>;

struct LabelOffset final
{
    bool        relative;
    size_t      offset;
    std::string label;
};

struct LexerContext;

struct Context final : utils::Immobile
{
    using Data = std::vector<uint8_t>;
    using LabelOffsets = std::vector<LabelOffset>;
    using LabelToAddress = std::map<std::string, uint32_t>;

    size_t          dataOffset;
    Data            rom;
    LexerContext*   lexerContext;
    LabelToAddress  labelToAddress;
    LabelOffsets    labelOffsetTable;
    UserSection*    currentUserSection;
    Section         sections[1];
    UserSections    userSections;
    Errors          errors;
};

}  // namespace assembler
