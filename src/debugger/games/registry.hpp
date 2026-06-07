#pragma once

#include <string_view>

#include "debugger/games/game.hpp"

namespace debugger::games
{

struct Registry final
{
    Registry() = delete;

    using CheckFnPtr = bool (*)(const std::string_view& name);
    using CreateFnPtr = GamePtr (*)();

    struct Registered final
    {
        const CheckFnPtr check;
        const CreateFnPtr create;
    };

    static void registerGame(
        CheckFnPtr check,
        CreateFnPtr create);

    static GamePtr createGame(std::string_view title);
};

#define REGISTER_GAME(CLASS, ...) \
    struct CLASS ## Registrator final \
    { \
        static void init() \
        { \
            Registry::registerGame(&check, &create); \
        } \
        static bool check(const std::string_view& title) \
        { \
            constexpr const char* names[] = {__VA_ARGS__}; \
            for (const auto name : names) \
            { \
                if (name == title) \
                { \
                    return true; \
                } \
            } \
            return false; \
        } \
        static GamePtr create() \
        { \
            return utils::makeUnique<CLASS>(); \
        } \
        static inline bool registered = (init(), true); \
    }

}  // namespace debugger::games
