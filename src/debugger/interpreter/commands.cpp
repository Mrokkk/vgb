#include "commands.hpp"

#include <map>

#include "debugger/interpreter/command.hpp"

namespace debugger::interpreter
{

struct CommandsMap
{
    static std::map<std::string_view, Command>& get()
    {
        static std::map<std::string_view, Command> commandsMap;
        return commandsMap;
    }
};

Command* Commands::find(const std::string_view& name)
{
    const auto it = CommandsMap::get().find(name);
    return it != CommandsMap::get().end()
        ? &it->second
        : nullptr;
}

void Commands::forEach(utils::FunctionRef<void(const Command&)> callback)
{
    for (const auto& command : CommandsMap::get())
    {
        callback(command.second);
    }
}

void Commands::$register(Command command)
{
    CommandsMap::get().emplace(command.name, std::move(command));
}

}  // namespace debugger::interpreter
