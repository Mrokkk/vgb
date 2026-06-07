#include "registry.hpp"

#include <string_view>
#include <vector>

namespace debugger::games
{

struct RegistryData
{
    static std::vector<Registry::Registered>& get()
    {
        static std::vector<Registry::Registered> data;
        return data;
    }
};

void Registry::registerGame(
    CheckFnPtr check,
    CreateFnPtr create)
{
    auto& data = RegistryData::get();
    data.push_back(Registered{
        .check = check,
        .create = create
    });
}

GamePtr Registry::createGame(std::string_view title)
{
    const auto& data = RegistryData::get();
    for (const auto& game : data)
    {
        if (game.check(title))
        {
            return game.create();
        }
    }
    return nullptr;
}

}  // namespace debugger::games
