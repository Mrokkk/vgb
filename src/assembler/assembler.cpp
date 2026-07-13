#include "assembler.hpp"

#include <string_view>

#include "assembler/context.hpp"
#include "assembler/glue.hpp"
#include "assembler/helpers.hpp"
#include "assembler/lexer.hpp"
#include "utils/byte_order.hpp"
#include "utils/units.hpp"

using namespace utils::literals;

namespace assembler
{

MaybeRom assemble(const std::string_view& fileName)
{
    Context ctx{
        .dataOffset = 0,
        .rom{},
        .lexerContext = nullptr
    };

    ctx.rom.resize(32_KiB);

    parse(fileName, ctx);

    if (not ctx.errors.empty()) [[unlikely]]
    {
        goto error;
    }

    for (const auto& e : ctx.labelOffsetTable)
    {
        const auto& label = e.label;
        const auto offset = e.offset;

        const auto labelAddress = findValue(ctx.labelToAddress, label);

        if (not labelAddress)
        {
            reportError(ctx, "undefined reference to {}", label);
            continue;
        };

        if (e.relative)
        {
            const long relative = static_cast<long>(*labelAddress) - static_cast<long>(offset) - 1;
            if (relative > INT8_MAX or relative < INT8_MIN)
            {
                reportError(ctx, "cannot perform relative jump, offset is too long ({})", relative);
                continue;
            }
            ctx.rom[offset] = static_cast<int8_t>(relative);
        }
        else
        {
            ctx.rom[offset] = utils::lsb(*labelAddress);
            ctx.rom[offset + 1] = utils::msb(*labelAddress);
        }
    }

    if (not ctx.errors.empty()) [[unlikely]]
    {
        goto error;
    }

    return std::move(ctx.rom);

error:
    return std::unexpected(std::move(ctx.errors));
}

}  // namespace assembler
