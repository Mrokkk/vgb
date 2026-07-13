#include <string_view>

#include <fmt/base.h>

#include "assembler/context.hpp"
#include "assembler/glue.hpp"
#include "assembler/lexer.hpp"

namespace assembler
{

void reportError(Context& ctx, const std::string_view& s)
{
    ctx.errors.emplace_back(Error{
        .message{std::string(s)},
        .location{getCurrentLocation(&ctx)}
    });
}

}  // namespace assembler
