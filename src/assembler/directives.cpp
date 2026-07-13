#include "assembler/context.hpp"
#include "assembler/glue.hpp"

namespace assembler
{

void handleSectionDirective(Context& ctx, const std::string_view& name, SectionType, uint16_t address)
{
    if (ctx.currentUserSection)
    {
        ctx.currentUserSection->currentOffset = ctx.dataOffset;
    }

    auto result = ctx.userSections.emplace(name, UserSection{.section = &ctx.sections[0], .currentOffset = address});

    if (not result.second)
    {
        reportError(ctx, "section \"{}\" already defined", name);
    }

    ctx.currentUserSection = &result.first->second;
    ctx.dataOffset = address;
}

}  // namespace assembler
