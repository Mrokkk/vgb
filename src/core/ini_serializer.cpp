#include "ini_serializer.hpp"

#include <charconv>
#include <map>
#include <sstream>
#include <string>

#include <fmt/fmt_ext.h>
#include <system_error>

namespace core
{

namespace
{

using Type = IniSerializer::Type;

struct DataEntry final
{
    const Type             type;
    const std::string_view name;
    void* const            data;
};

struct Registry final
{
    using Map = std::map<std::string_view, DataEntry>;

    static Map& get()
    {
        static Map map;
        return map;
    }
};

}  // namespace

void IniSerializer::registerData(const std::string_view& name, void* data, Type type)
{
    auto& registry = Registry::get();
    registry.emplace(
        name,
        DataEntry{
            .type = type,
            .name = name,
            .data = data,
        });
}

std::string IniSerializer::serialize()
{
    std::stringstream ss;
    for (const auto& [_, entry] : Registry::get())
    {
        ss << entry.name << '=';
        switch (entry.type)
        {
            case Type::Bool:
                ss << *static_cast<const bool*>(entry.data);
                break;
            case Type::Int32:
                ss << *static_cast<const int32_t*>(entry.data);
                break;
            case Type::Uint32:
                ss << *static_cast<const uint32_t*>(entry.data);
                break;
            case Type::String:
                ss << *static_cast<const std::string*>(entry.data);
                break;
        }
        ss << '\n';
    }
    return ss.str();
}

DeserializationResult IniSerializer::deserializeLine(std::string_view line)
{
    auto eq = line.find('=');

    if (eq == line.npos) [[unlikely]]
    {
        return std::unexpected(fmt::format_to_string("Missing \"=\": {}", line));
    }

    auto name = line.substr(0, eq);

    auto& registry = Registry::get();
    auto it = registry.find(name);

    if (it == registry.end())
    {
        return std::unexpected(fmt::format_to_string("Unexpected name: {}", name));
    }

    line.remove_prefix(eq + 1);

    auto& entry = it->second;

#define CONVERT_INT(TYPE) \
    ({ \
        TYPE value{0}; \
        auto result = std::from_chars(line.data(), line.data() + line.size(), value, 10); \
        if (result.ec == std::errc::invalid_argument) [[unlikely]] \
        { \
            return std::unexpected(fmt::format_to_string("Not an integer: {}", line)); \
        } \
        else if (result.ec == std::errc::result_out_of_range) \
        { \
            return std::unexpected(fmt::format_to_string("Out of range: {}", line)); \
        } \
        value; \
    })

    switch (entry.type)
    {
        case Type::Bool:
        {
            auto value = CONVERT_INT(uint8_t);
            if (value != 0 and value != 1)
            {
                return std::unexpected(fmt::format_to_string("Invalid value for bool: {}", line));
            }
            *static_cast<bool*>(entry.data) = value;
            break;
        }

        case Type::Int32:
        {
            auto value = CONVERT_INT(int32_t);
            *static_cast<int32_t*>(entry.data) = value;
            break;
        }

        case Type::Uint32:
        {
            auto value = CONVERT_INT(uint32_t);
            *static_cast<uint32_t*>(entry.data) = value;
            break;
        }

        case Type::String:
        {
            *static_cast<std::string*>(entry.data) = line;
            break;
        }

        default:
            break;
    }

    return true;
}

}  // namespace core
