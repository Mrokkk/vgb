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
    requires (std::is_pod_v<T> and not std::is_pointer_v<T> and not std::is_array_v<T>)
    ALWAYS_INLINE static void registerData(T& data)
    {
        registerData(reinterpret_cast<void*>(&data), sizeof(T));
    }

    template <typename T, size_t Size>
    ALWAYS_INLINE static void registerData(T (&data)[Size])
    {
        registerData(reinterpret_cast<void*>(data), Size);
    }

    static void registerData(void* data, size_t size);
    static void registerEvents(std::vector<Event*> events);
    static bool removeData(void* data);

    static size_t getDataSize();

    static SerializationResult   serialize();
    static DeserializationResult deserialize(const void* data, size_t size);
};
