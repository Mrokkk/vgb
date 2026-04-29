#include "main.hpp"

#include <cstdio>
#include <expected>
#include <iterator>
#include <map>
#include <unistd.h>

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>

#include "cpu/sm83.hpp"
#include "debugger/interpreter/command.hpp"
#include "debugger/interpreter/commands.hpp"
#include "debugger/interpreter/lexer.hpp"
#include "debugger/interpreter/lexer_printers.hpp"
#include "debugger/printer.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"
#include "sys/system.hpp"
#include "utils/string.hpp"

namespace fmt
{

template <typename ...Args>
std::string format_to_string(format_string<Args...> fmt, Args&&... args)
{
    std::string tmp;
    format_to(std::back_inserter(tmp), fmt, std::forward<Args>(args)...);
    return tmp;
}

}  // namespace fmt

namespace debugger
{

std::map<std::string_view, std::string_view> aliases = {
    {"b",    "break"},
    {"c",    "continue"},
    {"cont", "continue"},
    {"regs", "registers"},
    {"r",    "registers"},
    {"s",    "step"},
};

static std::expected<interpreter::Arguments, std::string> getArgs(const interpreter::Tokens& tokens)
{
    interpreter::Arguments args;

    for (size_t i = 1; i < tokens.size(); ++i)
    {
        const auto& token = tokens[i];
        switch (token.type)
        {
            case interpreter::Token::Type::IntLiteral:
                args.emplace_back(token.value | utils::to<long>);
                break;

            case interpreter::Token::Type::Identifier:
                args.emplace_back(token.value);
                break;

            case interpreter::Token::Type::BooleanLiteral:
                args.emplace_back(token.value == "true");
                break;

            case interpreter::Token::Type::Newline:
            case interpreter::Token::Type::Whitespace:
            case interpreter::Token::Type::End:
                continue;

            default:
            {
                std::string out;
                fmt::format_to(std::back_inserter(out), "Invalid syntax: unexpected token {}", token.value);
                return std::unexpected(std::move(out));
            }
        }
    }

    return args;
}

struct CommandData
{
    const interpreter::Command& command;
    interpreter::Arguments      args;
};

static std::expected<CommandData, std::string> parseCommand(const std::string_view& line)
{
    auto result = interpreter::parse(std::string(line));

    if (not result) [[unlikely]]
    {
        return std::unexpected(fmt::format_to_string("Invalid syntax: {}", result.error()));
    }

    const auto& tokens = result.value();

    if (tokens[0].type != interpreter::Token::Type::Identifier) [[unlikely]]
    {
        return std::unexpected(fmt::format_to_string("Invalid syntax: identifier expected, got \"{}\"", tokens[0].value));
    }

    auto commandStr = tokens[0].value;

    if (aliases.contains(commandStr))
    {
        commandStr = aliases[commandStr];
    }

    const auto command = interpreter::commands.find(commandStr);

    if (not command) [[unlikely]]
    {
        return std::unexpected("No such command");
    }

    auto maybeArgs = getArgs(tokens);

    if (not maybeArgs) [[unlikely]]
    {
        return std::unexpected(std::move(maybeArgs.error()));
    }

    return CommandData{
        .command = *command,
        .args = *maybeArgs
    };
}

void runCpu(State& state)
{
    while (1)
    {
        if (state.cpu.stopped)
        {
            state.stopped = true;
            state.cpu.stopped = false;
            break;
        }
        if ((int)state.cpu.pc != state.prevBreakpoint)
        {
            const auto breakpoint = state.breakpoints.find(state.cpu.pc);

            if (breakpoint != state.breakpoints.end())
            {
                fmt::println("Hit breakpoint {} at {:#04x}", breakpoint->second.id, breakpoint->second.address);
                state.prevBreakpoint = state.cpu.pc;
                state.stopped = true;
                break;
            }
        }

        if (state.cpu.step())
        {
            break;
        }
    }
    printInstruction(state.cpu);
}

void main()
{
    fmt::println("Debugger mode");
    fmt::println("For help, type \"help\"");

    sys::stopSupervision();

    State state{
        .cpu = gb.cpu,
        .stopped = true,
        .printRegs = false,
        .prevBreakpoint = -1,
        .prompt = fmt::format_to_string("{} ", fmt::styled("(vgb)", fmt::fg(fmt::terminal_color::cyan))),
    };

    while (1)
    {
        auto res = sys::readLineFromStdin(state.prompt);

        if (not res) [[unlikely]]
        {
            fmt::println("Error reading from stdin: {}", res.error());
            break;
        }
        else if (res->empty())
        {
            fmt::println("");
            break;
        }
        else if (res->size() == 1 and res->at(0) == '\n')
        {
            if (state.prevLine.empty()) [[unlikely]]
            {
                continue;
            }
            res = state.prevLine;
        }

        auto parsed = parseCommand(*res);

        if (not parsed) [[unlikely]]
        {
            fmt::println("{}", parsed.error());
            continue;
        }

        if (parsed->command.handler(state, parsed->args))
        {
            continue;
        }

        state.prevLine = *res;

        if (not state.stopped)
        {
            runCpu(state);
        }
    }
}

}  // namespace debugger
