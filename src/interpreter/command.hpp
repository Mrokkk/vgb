#pragma once

#include <cstdint>
#include <expected>
#include <vector>
#include <string_view>

#include "interpreter/operations.hpp"
#include "utils/inline.hpp"

namespace interpreter
{

enum class Type : uint8_t
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

#define EXECUTE_ARGUMENTS() \
        [[maybe_unused]] void* userData, \
        [[maybe_unused]] ::interpreter::Operations& ops, \
        [[maybe_unused]] const ::interpreter::Arguments& args

using ExecutionResult = std::expected<bool, std::string>;
using Arguments = std::vector<Argument>;
using CommandHandler = ExecutionResult (*)(EXECUTE_ARGUMENTS());

struct Command final
{
    const std::string_view name;
    const CommandHandler   handler;
    const std::string_view help;
    void* const            userData;
};

struct CommandBase
{
    CommandBase() = delete;
    static void $register(Command command);
};

#define EXECUTOR() \
    ::interpreter::ExecutionResult Command::execute(EXECUTE_ARGUMENTS())

#define GET_ARGUMENT(ID, TYPE) \
    ({ \
        if (args.size() < ID + 1) [[unlikely]] \
        { \
            return std::unexpected("Invalid number of arguments"); \
        } \
        auto arg = args[ID].get ## TYPE(); \
        if (not arg) [[unlikely]] \
        { \
            return std::unexpected("Invalid type of argument"); \
        } \
        *arg; \
    })

#define GET_USER_DATA(TYPE) \
    *static_cast<TYPE*>(userData)

#define ARGC() \
    args.size()

#define HELP() \
    const char* Command::help

#define DEFINE_COMMAND_IMPL(NAME, ...) \
    namespace _##NAME \
    { \
    struct Command : ::interpreter::CommandBase \
    { \
        static void registerCommand(void* userData = nullptr) \
        { \
            ::interpreter::CommandBase::$register( \
                ::interpreter::Command{ \
                    .name = name, \
                    .handler = &Command::execute, \
                    .help = Command::help, \
                    .userData = userData \
                }); \
        } \
    private: \
        static ::interpreter::ExecutionResult execute(EXECUTE_ARGUMENTS()); \
        static const char* help; \
        static inline const char* name = #NAME; \
        __VA_ARGS__; \
    }; \
    } \
    namespace _##NAME

#define DEFINE_COMMAND(NAME) \
    DEFINE_COMMAND_IMPL(NAME)

#define DEFINE_AND_REGISTER_COMMAND(NAME) \
    DEFINE_COMMAND_IMPL(NAME, static inline bool registered = (Command::registerCommand(nullptr), true))

#define COMMAND(NAME) _##NAME::Command

}  // namespace interpreter
