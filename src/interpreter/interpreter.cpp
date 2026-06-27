#include "interpreter.hpp"

#include <map>

#include <fmt/fmt_ext.h>

#include "interpreter/command.hpp"
#include "interpreter/lexer.hpp"
#include "interpreter/state.hpp"
#include "utils/string.hpp"

namespace interpreter
{

namespace
{

struct CommandData final
{
    const interpreter::Command&  command;
    const interpreter::Arguments args;
};

struct CommandsMap final
{
    static std::map<std::string_view, Command>& get()
    {
        static std::map<std::string_view, Command> commandsMap;
        return commandsMap;
    }
};

}  // namespace

static const Command* findCommand(const std::string_view& name)
{
    const auto& map = CommandsMap::get();
    const auto it = map.find(name);
    return it != map.end()
        ? &it->second
        : nullptr;
}

using MaybeCommandData = std::expected<CommandData, std::string>;
using MaybeArguments = std::expected<Arguments, std::string>;

static State state;
static std::map<std::string, std::string, std::less<>> aliases;

static MaybeArguments getArgs(const Tokens& tokens)
{
    interpreter::Arguments args;
    int mult = 1;

    for (size_t i = 1; i < tokens.size(); ++i)
    {
        const auto& token = tokens[i];
        switch (token.type)
        {
            case interpreter::Token::Type::Sub:
                if (i + 1 >= tokens.size() or tokens[i + 1].type != interpreter::Token::Type::IntLiteral)
                {
                    goto error;
                }
                mult = -1;
                continue;

            case interpreter::Token::Type::IntLiteral:
                args.emplace_back(mult * (token.value | utils::to<long>));
                mult = 1;
                break;

            case interpreter::Token::Type::Identifier:
            case interpreter::Token::Type::StringLiteral:
                args.emplace_back(token.value);
                break;

            case interpreter::Token::Type::BooleanLiteral:
                args.emplace_back(token.value == "true");
                break;

            case interpreter::Token::Type::Newline:
            case interpreter::Token::Type::Whitespace:
            case interpreter::Token::Type::End:
                continue;

            error:
            default:
                return std::unexpected(fmt::format_to_string("Invalid syntax: unexpected token {}", token.value));
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

    if (auto it = aliases.find(commandStr); it != aliases.end())
    {
        commandStr = it->second;
    }

    const auto command = findCommand(commandStr);

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

void initialize(Operations ops, void* defaultUserData)
{
    state.ops = std::move(ops);
    state.defaultUserData = defaultUserData;
}

void setAlias(std::string alias, std::string command)
{
    aliases[std::move(alias)] = std::move(command);
}

void forEachAlias(utils::FunctionRef<void(const std::string&, const std::string&)> callback)
{
    for (const auto& [alias, command] : aliases)
    {
        callback(alias, command);
    }
}

void forEachCommand(utils::FunctionRef<void(const Command&)> callback)
{
    for (const auto& command : CommandsMap::get())
    {
        callback(command.second);
    }
}

void registerCommand(Command command)
{
    CommandsMap::get().emplace(command.name, std::move(command));
}

ExecutionResult exectuteCommand(std::string command)
{
    state.currentCommand = std::move(command);
    auto parsed = parseCommand(state.currentCommand);

    if (not parsed) [[unlikely]]
    {
        return std::unexpected(std::move(parsed.error()));
    }

    auto userData = parsed->command.userData
        ? parsed->command.userData
        : state.defaultUserData;

    return parsed->command.handler(userData, state.ops, parsed->args);
}

}  // namespace interpreter
