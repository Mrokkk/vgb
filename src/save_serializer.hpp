#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <type_traits>
#include <vector>

#include "utils/inline.hpp"

struct Event;

using SerializedData = std::vector<uint8_t>;
using SerializationResult = std::expected<SerializedData, std::string>;
using DeserializationResult = std::expected<std::nullptr_t, std::string>;

struct SaveSerializer
{
    template <typename T>
    requires (std::is_standard_layout_v<T> and not std::is_pointer_v<T> and not std::is_array_v<T>)
    ALWAYS_INLINE static void registerData(const std::string_view& name, T& data)
    {
        registerData(name, reinterpret_cast<void*>(&data), sizeof(T));
    }

    template <typename T, size_t Size>
    ALWAYS_INLINE static void registerData(const std::string_view& name, T (&data)[Size])
    {
        registerData(name, reinterpret_cast<void*>(data), Size);
    }

    static void registerData(const std::string_view& name, void* data, size_t size);
    static void registerData(const std::string_view& name, Event& event);
    static void removeData(const std::string_view& name);

    static size_t getDataSize();

    static SerializationResult   serialize();
    static DeserializationResult deserialize(const void* data, size_t size);
};
