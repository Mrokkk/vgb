#pragma once

#include <string_view>

#include <fmt/format.h>

#include "assembler/argument.hpp"
#include "assembler/context.hpp"
#include "assembler/error.hpp"
#include "cpu/isa/opcode.hpp"

namespace assembler
{

void handleInstruction(Context& ctx, cpu::isa::Opcode::Type mnemo, const Arguments& args);
void handleLabel(Context& ctx, const std::string_view& label);
void handleSectionDirective(Context& ctx, const std::string_view& name, SectionType type, uint16_t address);
void handleIncludeDirective(Context& ctx, const std::string_view& name);
void reportError(Context& context, const std::string_view& s);

template <typename ...Args>
void reportError(Context& ctx, fmt::format_string<Args...> fmt, Args&&... args)
{
    ctx.errors.emplace_back(Error{
        .message{fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...)},
        .location{getCurrentLocation(&ctx)}
    });
}

}  // namespace assembler
