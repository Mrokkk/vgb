#pragma once

#include <algorithm>
#include <cstdint>
#include <expected>
#include <string_view>

#include "utils/inline.hpp"

namespace core
{

using DeserializationResult = std::expected<bool, std::string>;

struct IniSerializer final
{
    enum class Type
    {
        Bool,
        Int32,
        Uint32,
        String,
    };

    ALWAYS_INLINE static void registerData(const std::string_view& name, bool& data)
    {
        registerData(name, &data, Type::Bool);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, int32_t& data)
    {
        registerData(name, &data, Type::Int32);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, uint32_t& data)
    {
        registerData(name, &data, Type::Uint32);
    }

    static void registerData(const std::string_view& name, void* data, Type type);

    static std::string serialize();
    static DeserializationResult deserializeLine(std::string_view line);
};

template <std::size_t N>
struct ConstevalString
{
    consteval ConstevalString(const char (&string)[N])
    {
        std::copy_n(string, N, data);
    }

    char data[N];
};

template <typename T, ConstevalString Name>
struct IniSaved final
{
    constexpr IniSaved()
        : mValue()
    {
        IniSerializer::registerData(Name.data, mValue);
    }

    ALWAYS_INLINE constexpr operator T&()             { return mValue; }
    ALWAYS_INLINE constexpr operator const T&() const { return mValue; }

    ALWAYS_INLINE constexpr T& operator=(const T& value) { return mValue = value; }
    ALWAYS_INLINE constexpr T& operator=(T&& value)      { return mValue = std::move(value); }

    ALWAYS_INLINE constexpr T& get() { return mValue; }

private:
    T mValue;
};

#define INI_SAVED(TYPE, NAME) \
    ::core::IniSaved<TYPE, #NAME> NAME

}  // namespace core
