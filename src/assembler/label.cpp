#include <string>
#include <string_view>

#include "assembler/context.hpp"
#include "assembler/glue.hpp"

namespace assembler
{

void handleLabel(Context& ctx, const std::string_view& label)
{
    auto res = ctx.labelToAddress.emplace(std::string(label), ctx.dataOffset);

    if (not res.second)
    {
        reportError(ctx, "label {} was already defined", label);
    }
}

}  // namespace assembler
