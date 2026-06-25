#include "interpreter.hpp"

#include <map>

#include "debugger/interpreter/command.hpp"
#include "debugger/interpreter/commands.hpp"
#include "debugger/interpreter/lexer.hpp"
#include "utils/string.hpp"

namespace debugger::interpreter
{

struct CommandData final
{
    const interpreter::Command& command;
    interpreter::Arguments      args;
};

using MaybeCommandData = std::expected<CommandData, std::string>;
using MaybeArguments = std::expected<Arguments, std::string>;

static const std::map<std::string_view, std::string_view> aliases = {
    {"b",    "break"},
    {"c",    "continue"},
    {"cont", "continue"},
    {"regs", "registers"},
    {"r",    "registers"},
    {"s",    "step"},
};

static MaybeArguments getArgs(const interpreter::Tokens& tokens)
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

static MaybeCommandData parseCommand(const std::string_view& line)
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

    const auto command = commands.find(commandStr);

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

std::expected<std::nullptr_t, std::string> exectuteCommand(std::string command, State& state)
{
    auto parsed = parseCommand(command);

    if (not parsed) [[unlikely]]
    {
        return std::unexpected(std::move(parsed.error()));
    }
    else
    {
        parsed->command.handler(state, parsed->args);
    }

    return {};
}

}  // namespace debugger::interpreter
