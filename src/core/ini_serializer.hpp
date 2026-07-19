#pragma once

#include <algorithm>
#include <cstdint>
#include <expected>
#include <functional>
#include <string_view>

#include "utils/consteval_string.hpp"
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
        Float,
        String,
    };

    ALWAYS_INLINE static void registerData(const std::string_view& name, bool& data)
    {
        registerData(name, &data, 1, Type::Bool);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, int32_t& data)
    {
        registerData(name, &data, 1, Type::Int32);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, uint32_t& data)
    {
        registerData(name, &data, 1, Type::Uint32);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, float& data)
    {
        registerData(name, &data, 1, Type::Float);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, std::string& data)
    {
        registerData(name, &data, 1, Type::String);
    }

    ALWAYS_INLINE static void registerData(const std::string_view& name, uint32_t* data, size_t size)
    {
        registerData(name, data, size, Type::Uint32);
    }

    static void registerData(const std::string_view& name, void* data, size_t size, Type type);

    static std::string serialize();
    static DeserializationResult deserializeLine(std::string_view line);

    static void onLoaded(const std::string_view& name, std::move_only_function<void()> callback);
};

template <typename T, utils::ConstevalString Name>
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

template <typename T, size_t Size, utils::ConstevalString Name>
struct IniSavedArray final
{
    constexpr IniSavedArray()
        : mValues()
    {
        IniSerializer::registerData(Name.data, mValues, Size);
    }

    ALWAYS_INLINE constexpr operator T*()             { return mValues; }
    ALWAYS_INLINE constexpr operator const T*() const { return mValues; }

    ALWAYS_INLINE constexpr T* get() { return mValues; }

    ALWAYS_INLINE constexpr T& operator[](size_t i) { return mValues[i]; }

    const T* begin() const { return mValues; }
    const T* end() const { return mValues + Size; }

    T* begin() { return mValues; }
    T* end() { return mValues + Size; }

private:
    T mValues[Size];
};

#define INI_SAVED(TYPE, NAME) \
    ::core::IniSaved<TYPE, #NAME> NAME

#define INI_SAVED_ARRAY(TYPE, COUNT, NAME) \
    ::core::IniSavedArray<TYPE, COUNT, #NAME> NAME

}  // namespace core
