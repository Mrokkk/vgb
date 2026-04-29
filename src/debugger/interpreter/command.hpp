#pragma once

#include <fmt/base.h> // IWYU pragma: keep
#include <string_view>
#include <vector>

#include "debugger/state.hpp"
#include "utils/inline.hpp"

namespace debugger::interpreter
{

enum class Type
{
    String,
    Integer,
    Boolean,
};

struct Argument
{
    ALWAYS_INLINE explicit Argument(std::string_view string)
        : type(Type::String)
        , mString(std::move(string))
    {
    }

    ALWAYS_INLINE explicit Argument(long integer)
        : type(Type::Integer)
        , mInteger(integer)
    {
    }

    ALWAYS_INLINE explicit Argument(bool boolean)
        : type(Type::Boolean)
        , mBoolean(boolean)
    {
    }

#define ARG_CHECKER(TYPE) \
    ALWAYS_INLINE bool is ## TYPE() const \
    { \
        return type == Type::TYPE; \
    }

    ARG_CHECKER(Integer)
    ARG_CHECKER(String)
    ARG_CHECKER(Boolean)

#define ARG_GETTER(CTYPE, TYPE) \
    ALWAYS_INLINE const CTYPE* get ## TYPE() const \
    { \
        return is ## TYPE() ? &m ## TYPE : nullptr; \
    }

    ARG_GETTER(std::string_view, String)
    ARG_GETTER(long, Integer)
    ARG_GETTER(bool, Boolean)

    Type type;
    union
    {
        std::string_view mString;
        long             mInteger;
        bool             mBoolean;
    };
};

using Arguments = std::vector<Argument>;

using CommandHandler = int (*)(State& state, const Arguments& arguments);

struct Command
{
    std::string_view name;
    CommandHandler   handler;
    std::string_view help;
};

struct CommandBase
{
    static void $register(Command command);
};

#define EXECUTOR() \
    int Command::execute( \
        [[maybe_unused]] ::debugger::State& state, \
        [[maybe_unused]] const ::debugger::interpreter::Arguments& args)

#define ARGUMENT_GET(ID, TYPE) \
    ({ \
        if (args.size() < ID + 1) [[unlikely]] \
        { \
            fmt::println("Invalid number of arguments"); \
            return 1; \
        } \
        auto arg = args[ID].get ## TYPE(); \
        if (not arg) [[unlikely]] \
        { \
            fmt::println("Invalid type of argument"); \
            return 1; \
        } \
        *arg; \
    })

#define HELP() \
    const char* Command::help

#define DEFINE_COMMAND(NAME) \
    namespace _##NAME \
    { \
    struct Command : ::debugger::interpreter::CommandBase \
    { \
        static int execute(::debugger::State& state, const ::debugger::interpreter::Arguments& arguments); \
        static void init() \
        { \
            $register( \
                ::debugger::interpreter::Command{ \
                    .name = #NAME, \
                    .handler = &Command::execute, \
                    .help = Command::help \
                }); \
        } \
        static const char* help; \
        static inline const char* name = #NAME; \
        static inline bool registered = (Command::init(), true); \
    }; \
    } \
    namespace _##NAME


}  // namespace debugger::interpreter
