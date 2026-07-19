#include "ini_serializer.hpp"

#include <charconv>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <system_error>

#include <fmt/format.h>

namespace core
{

namespace
{

using Type = IniSerializer::Type;

struct DataEntry final
{
    const Type             type;
    const size_t           count;
    void* const            data;
    const std::string_view name;
};

struct Registry final
{
    using Callbacks = std::map<std::string_view, std::move_only_function<void()>>;
    using Map = std::map<std::string_view, DataEntry>;

    static Callbacks& getCallbacks()
    {
        static Callbacks cbs;
        return cbs;
    }

    static Map& get()
    {
        static Map map;
        return map;
    }
};

template <typename T>
ALWAYS_INLINE T shift(T ptr, ssize_t off)
{
    return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(ptr) + off);
}

}  // namespace

void IniSerializer::registerData(const std::string_view& name, void* data, size_t size, Type type)
{
    auto& registry = Registry::get();
    registry.emplace(
        name,
        DataEntry{
            .type = type,
            .count = size,
            .data = data,
            .name = name,
        });
}

std::string IniSerializer::serialize()
{
    std::stringstream ss;

#define SERIALIZE(ENUM, TYPE) \
    case Type::ENUM: \
        ss << *static_cast<const TYPE*>(data); \
        data = shift(data, sizeof(TYPE)); \
        break

    for (const auto& [_, entry] : Registry::get())
    {
        ss << entry.name << '=';
        const void* data = entry.data;
        for (size_t i = 0; i < entry.count; ++i)
        {
            switch (entry.type)
            {
                SERIALIZE(Bool, bool);
                SERIALIZE(Int32, int32_t);
                SERIALIZE(Uint32, uint32_t);
                SERIALIZE(Float, float);
                SERIALIZE(String, std::string);
            }
            if (i != entry.count - 1)
            {
                ss << ',';
            }
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
        return std::unexpected(fmt::format("Missing \"=\": {}", line));
    }

    auto name = line.substr(0, eq);

    auto& registry = Registry::get();
    auto it = registry.find(name);

    if (it == registry.end())
    {
        return std::unexpected(fmt::format("Unexpected name: {}", name));
    }

    line.remove_prefix(eq + 1);

    auto& entry = it->second;

#define CONVERT_NUMBER(TYPE) \
    ({ \
        TYPE value{0}; \
        auto result = std::from_chars(toConvert.data(), toConvert.data() + toConvert.size(), value); \
        if (result.ec == std::errc::invalid_argument) [[unlikely]] \
        { \
            return std::unexpected(fmt::format("Not an integer: {}", toConvert)); \
        } \
        else if (result.ec == std::errc::result_out_of_range) \
        { \
            return std::unexpected(fmt::format("Out of range: {}", toConvert)); \
        } \
        value; \
    })

    void* data = entry.data;

    for (size_t i = 0; i < entry.count; ++i)
    {
        auto comma = line.find(',');
        if (comma == line.npos)
        {
            if (i != entry.count - 1)
            {
                return std::unexpected(fmt::format("Expected \",\": {}", line));
            }
            comma = line.size();
        }
        auto toConvert = line.substr(0, comma);
        switch (entry.type)
        {
            case Type::Bool:
            {
                auto value = CONVERT_NUMBER(uint8_t);
                if (value != 0 and value != 1)
                {
                    return std::unexpected(fmt::format("Invalid value for bool: {}", line));
                }
                *static_cast<bool*>(data) = value;
                data = shift(data, sizeof(bool));
                break;
            }

            case Type::Int32:
            {
                auto value = CONVERT_NUMBER(int32_t);
                *static_cast<int32_t*>(data) = value;
                data = shift(data, sizeof(int32_t));
                break;
            }

            case Type::Uint32:
            {
                auto value = CONVERT_NUMBER(uint32_t);
                *static_cast<uint32_t*>(data) = value;
                data = shift(data, sizeof(uint32_t));
                break;
            }

            case Type::Float:
            {
                auto value = CONVERT_NUMBER(float);
                *static_cast<float*>(data) = value;
                data = shift(data, sizeof(float));
                break;
            }

            case Type::String:
            {
                *static_cast<std::string*>(data) = toConvert;
                data = shift(data, sizeof(std::string));
                break;
            }

            default:
                break;
        }
        line.remove_prefix(comma + 1);
    }

    auto& callbacks = Registry::getCallbacks();
    if (auto it = callbacks.find(name); it != callbacks.end())
    {
        it->second();
    }

    return true;
}

void IniSerializer::onLoaded(const std::string_view& name, std::move_only_function<void()> callback)
{
    Registry::getCallbacks().emplace(name, std::move(callback));
}

}  // namespace core
