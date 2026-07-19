#include "save_serializer.hpp"

#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

#include <fmt/base.h>

#include "error.hpp"
#include "event.hpp"
#include "event_system.hpp"
#include "game_boy.hpp"
#include "utils/immobile.hpp"
#include "utils/inline.hpp"

namespace
{

constexpr uint16_t SIG = 0x4392;
constexpr size_t MAX_NAME_LEN = 128;

struct Reader final : utils::Immobile
{
    ALWAYS_INLINE constexpr Reader(const void* data, size_t size)
        : mData(static_cast<const uint8_t*>(data))
        , mSize(size)
        , mCur(0)
    {
    }

    int read(void* buffer, size_t size)
    {
        if (mCur + size > mSize) [[unlikely]]
        {
            return -1;
        }
        memcpy(buffer, mData + mCur, size);
        mCur += size;
        return 0;
    }

    int readAt(void* buffer, size_t off, size_t size)
    {
        if (validateReadAt(off, size)) [[unlikely]]
        {
            return -1;
        }
        memcpy(buffer, mData + off, size);
        return 0;
    }

    ALWAYS_INLINE int validateReadAt(size_t off, size_t size)
    {
        return off + size > mSize;
    }

    ALWAYS_INLINE size_t getCurrent() const
    {
        return mCur;
    }

private:
    const uint8_t* mData;
    const size_t   mSize;
    size_t         mCur;
};

struct Writer final : utils::Immobile
{
    ALWAYS_INLINE constexpr Writer(size_t size)
        : mData(size)
        , mCur(0)
    {
    }

    ALWAYS_INLINE SerializedData release()
    {
        return std::move(mData);
    }

    ALWAYS_INLINE void write(const void* data, size_t size)
    {
        memcpy(mData.data() + mCur, data, size);
        mCur += size;
    }

    template <typename T>
    requires (not std::is_pointer_v<T>)
    ALWAYS_INLINE void writeAt(const T* data, size_t off, size_t size)
    {
        memcpy(mData.data() + off, data, size);
    }

    ALWAYS_INLINE size_t size() const
    {
        return mData.size();
    }

    ALWAYS_INLINE size_t getCurrent() const
    {
        return mCur;
    }

private:
    SerializedData mData;
    size_t         mCur;
};

enum class DataType
{
    Data,
    Event,
};

struct DataEntry final
{
    DataType   type;
    union
    {
        void*  data;
        Event* event;
    };
    uint32_t   size;
};

struct SerializedEvent final
{
    uint64_t when;
    uint64_t period;
};

struct SerializedDataEntry final
{
    uint16_t sig;
    uint16_t entryLen;
    DataType type;
    uint32_t dataOffset;
    uint32_t dataSize;
    char     name[];
};

struct Registry final : utils::Immobile
{
    static Registry& instance()
    {
        static Registry s;
        return s;
    }

    void addDataEntry(const std::string_view& name, DataEntry e)
    {
        assertFormat(name.size() < MAX_NAME_LEN, "\"{}\": name is longer than {}", name, MAX_NAME_LEN);
        mNamesLen += name.size();
        mDataSize += e.size;
        auto res = mDataEntries.emplace(name, std::move(e));
        assertFormat(res.second, "entry for name {} was already added", name);
    }

    void removeDataEntry(const std::string_view& name)
    {
        auto it = mDataEntries.find(name);
        assertFormat(it != mDataEntries.end(), "entry for {} does not exist", name);
        auto& entry = it->second;
        mNamesLen -= name.size();
        mDataSize -= entry.size;
        mDataEntries.erase(it);
    }

    ALWAYS_INLINE size_t getStringsLen() const
    {
        return mNamesLen;
    }

    ALWAYS_INLINE size_t getDataSize() const
    {
        return mDataSize;
    }

    ALWAYS_INLINE auto& getDataEntries()
    {
        return mDataEntries;
    }

private:
    ALWAYS_INLINE constexpr Registry()
        : mNamesLen(0)
        , mDataSize(0)
    {
    }

    size_t mNamesLen;
    size_t mDataSize;
    std::map<std::string_view, DataEntry> mDataEntries;
};

size_t getDataEntriesSize(Registry& registry)
{
    return sizeof(SerializedDataEntry) * registry.getDataEntries().size()
        + registry.getStringsLen();
}

}  // namespace

void SaveSerializer::registerData(const std::string_view& name, void* data, size_t size)
{
    Registry::instance().addDataEntry(
        name,
        DataEntry{
            .type = DataType::Data,
            .data = data,
            .size = static_cast<uint32_t>(size),
        });
}

void SaveSerializer::registerData(const std::string_view& name, Event& event)
{
    Registry::instance().addDataEntry(
        name,
        DataEntry{
            .type = DataType::Event,
            .event = &event,
            .size = sizeof(SerializedEvent),
        });
}

void SaveSerializer::removeData(const std::string_view& name)
{
    Registry::instance().removeDataEntry(name);
}

size_t SaveSerializer::getDataSize()
{
    auto& registry = Registry::instance();

    return registry.getDataEntries().size() * sizeof(SerializedDataEntry)
        + registry.getStringsLen()
        + registry.getDataSize();
}

SerializationResult SaveSerializer::serialize()
{
    auto& registry = Registry::instance();
    Writer writer(getDataSize());

    const size_t initialDataOffset = getDataEntriesSize(registry);

    size_t dataOffset = initialDataOffset;

    for (const auto& [name, entry] : registry.getDataEntries())
    {
        const SerializedDataEntry e{
            .sig = SIG,
            .entryLen = static_cast<uint16_t>(sizeof(SerializedDataEntry) + name.size()),
            .dataOffset = static_cast<uint32_t>(dataOffset),
            .dataSize = entry.size,
        };

        dataOffset += entry.size;

        writer.write(&e, sizeof(e));
        writer.write(name.data(), name.size());

        switch (entry.type)
        {
            case DataType::Data:
                writer.writeAt(entry.data, e.dataOffset, entry.size);
                break;
            case DataType::Event:
                SerializedEvent ev{
                    .when = entry.event->isActive() ? entry.event->getWhen() : 0,
                    .period = entry.event->getPeriod(),
                };
                writer.writeAt(&ev, e.dataOffset, sizeof(ev));
                break;
        }
    }

    return writer.release();
}

DeserializationResult SaveSerializer::deserialize(const void* data, size_t size)
{
    Reader reader(data, size);
    auto& registry = Registry::instance();

    if (size != getDataSize()) [[unlikely]]
    {
        return error("incorrect save size");
    }

    struct DeserializationEntry final
    {
        const size_t     dataOffset;
        const size_t     dataSize;
        const DataEntry& dataEntry;
    };

    std::vector<DeserializationEntry> toDeserialize;
    toDeserialize.reserve(registry.getDataEntries().size());

    for (size_t i = 0; i < registry.getDataEntries().size(); ++i)
    {
        SerializedDataEntry e;

        if (reader.read(&e, sizeof(e))) [[unlikely]]
        {
            goto unexpectedEof;
        }

        if (e.sig != SIG) [[unlikely]]
        {
            return error("incorrect entry signature");
        }

        char name[MAX_NAME_LEN + 1];
        auto nameLen = e.entryLen - sizeof(e);

        if (nameLen > sizeof(name)) [[unlikely]]
        {
            return error("entry name too long");
        }

        if (reader.read(name, nameLen)) [[unlikely]]
        {
            goto unexpectedEof;
        }

        const std::string_view nameSv(name, nameLen);

        const auto entryIt = registry.getDataEntries().find(nameSv);

        if (entryIt == registry.getDataEntries().end()) [[unlikely]]
        {
            return error("unexpected entry name: {}", nameSv);
        }

        const auto& entry = entryIt->second;

        if (entry.size != e.dataSize) [[unlikely]]
        {
            return error("entry {} has incorrect data size: {}", nameSv, e.dataSize);
        }

        if (reader.validateReadAt(e.dataOffset, e.dataSize)) [[unlikely]]
        {
            goto unexpectedEof;
        }

        if (entry.type == DataType::Event and e.dataSize != sizeof(SerializedEvent)) [[unlikely]]
        {
            return error("incorrect event data size");
        }

        toDeserialize.emplace_back(DeserializationEntry{e.dataOffset, e.dataSize, entry});
    }

    if (reader.getCurrent() != getDataEntriesSize(registry)) [[unlikely]]
    {
        return error("size of entries is incorrect");
    }

    for (const auto& e : toDeserialize)
    {
        switch (e.dataEntry.type)
        {
            case DataType::Data:
                reader.readAt(e.dataEntry.data, e.dataOffset, e.dataSize);
                break;

            case DataType::Event:
                SerializedEvent ev;
                reader.readAt(&ev, e.dataOffset, e.dataSize);
                e.dataEntry.event->setPeriod(ev.period);
                if (ev.when)
                {
                    gb.events.scheduleEvent(*e.dataEntry.event, ev.when);
                }
                break;
        }
    }

    return {};

unexpectedEof:
    return error("data too short");
}
