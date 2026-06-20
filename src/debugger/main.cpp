#include "main.hpp"

#include <cstdio>
#include <cstring>
#include <expected>
#include <iterator>
#include <map>
#include <unistd.h>

#include "cpu/exception.hpp"
#include "cpu/printers.hpp"
#include "cpu/sm83.hpp"
#include "debugger/games/registry.hpp"
#include "debugger/imgui.hpp"
#include "debugger/interpreter/command.hpp"
#include "debugger/interpreter/commands.hpp"
#include "debugger/interpreter/lexer.hpp"
#include "debugger/interpreter/lexer_printers.hpp"
#include "debugger/parser.hpp"
#include "debugger/printer.hpp"
#include "debugger/state.hpp"
#include "game_boy.hpp"
#include "utils/string.hpp"

namespace debugger
{

static const std::map<std::string_view, std::string_view> aliases = {
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

std::expected<CommandData, std::string> parseCommand(const std::string_view& line)
{
    auto result = interpreter::parse(line);

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
        commandStr = aliases.at(commandStr);
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

static void runCpu(State& state)
{
    while (1)
    {
        if (gb.state == GameBoy::State::Stopped)
        {
            break;
        }
        if ((int)state.cpu.pc != state.prevBreakpoint)
        {
            const auto breakpoint = state.breakpoints.find(state.cpu.pc);

            if (breakpoint != state.breakpoints.end())
            {
                logToConsole(state, "Hit breakpoint {} at {:#04x}", breakpoint->second.id, breakpoint->second.address);
                state.prevBreakpoint = state.cpu.pc;
                gb.stop();
                break;
            }
            else
            {
                state.prevBreakpoint = -1;
            }
        }

        if (state.cpu.step())
        {
            gb.stop();
            break;
        }
    }
    if (state.cpu.exc)
    {
        logToConsole(state, "Exception raised: {}", state.cpu.exc);
    }
    printInstruction(state, state.cpu);
}

void main(GameBoy& gb)
{
    State state{
        .cpu = gb.cpu,
        .prevBreakpoint = -1,
        .prompt = "(vgb)",
    };

    initImGui(state);
    logToConsole(state, "Debugger mode");
    logToConsole(state, "For help, type \"help\"");

    gb.debuggerData = reinterpret_cast<void*>(&state);

    state.game = games::Registry::createGame(gb.cartridge.getTitle());

    while (1)
    {
        if (state.cpu.exc.type == cpu::Exception::UserInterruption)
        {
            break;
        }

        while (gb.state == GameBoy::State::Stopped)
        {
            gb.frame();

            if (state.cpu.exc.type == cpu::Exception::UserInterruption)
            {
                goto finish;
            }
        }

        runCpu(state);
    }

finish:
    gb.debuggerData = nullptr;
}

}  // namespace debugger
