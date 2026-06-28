#include "save_serializer.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <iterator>
#include <vector>

#include "event.hpp"
#include "event_system.hpp"
#include "game_boy.hpp"
#include "utils/immobile.hpp"
#include "utils/inline.hpp"

constexpr char HEADER[] = {'V', 'G', 'B', 'C'};

namespace
{

struct Reader final : utils::Immobile
{
    Reader(const void* data, size_t size)
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

private:
    const uint8_t* mData;
    const size_t   mSize;
    size_t         mCur;
};

struct Writer final : utils::Immobile
{
    constexpr Writer(size_t size)
        : mData(size)
        , mOffset(0)
    {
    }

    SerializedData release()
    {
        return std::move(mData);
    }

    template <typename T>
    requires (not std::is_pointer_v<T>)
    void write(const T* data, size_t size)
    {
        writeImpl(static_cast<const void*>(data), size);
    }

private:
    void writeImpl(const void* data, size_t size)
    {
        memcpy(mData.data() + mOffset, data, size);
        mOffset += size;
    }

    SerializedData mData;
    size_t         mOffset;
};

struct DataEntry final
{
    void*  data;
    size_t size;
};

struct Registry final : utils::Immobile
{
    static Registry& instance()
    {
        static Registry s;
        return s;
    }

    void addEvents(std::vector<Event*> events)
    {
        std::copy(events.begin(), events.end(), std::back_inserter(mEvents));
    }

    void addDataEntry(DataEntry e)
    {
        mDataEntries.push_back(e);
        mSize += e.size;
    }

    ALWAYS_INLINE auto& getDataEntries()
    {
        return mDataEntries;
    }

    ALWAYS_INLINE const auto& getEvents() const
    {
        return mEvents;
    }

    ALWAYS_INLINE size_t getSize()
    {
        return mSize + mEvents.size() * sizeof(uint64_t);
    }

private:
    Registry()
        : mSize(0)
    {
    }

    std::vector<DataEntry> mDataEntries;
    std::vector<Event*> mEvents;
    size_t mSize;
};

}  // namespace

void SaveSerializer::registerEvents(std::vector<Event*> events)
{
    Registry::instance().addEvents(std::move(events));
}

void SaveSerializer::registerData(void* data, size_t size)
{
    Registry::instance().addDataEntry(DataEntry{.data = data, .size = size});
}

bool SaveSerializer::removeData(void* data)
{
    auto& entries = Registry::instance().getDataEntries();
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        if (it->data == data)
        {
            entries.erase(it);
            return true;
        }
    }
    return false;
}

size_t SaveSerializer::getDataSize()
{
    return Registry::instance().getSize() + sizeof(HEADER);
}

SerializationResult SaveSerializer::serialize()
{
    auto& registry = Registry::instance();
    Writer writer(getDataSize());
    writer.write(HEADER, sizeof(HEADER));
    for (const auto& entry : registry.getDataEntries())
    {
        writer.write(entry.data, entry.size);
    }
    for (const auto event : registry.getEvents())
    {
        uint64_t when = event->isActive() ? event->getWhen() : 0;
        writer.write(&when, sizeof(when));
    }
    return writer.release();
}

DeserializationResult SaveSerializer::deserialize(const void* data, size_t size)
{
    Reader reader(data, size);
    char header[sizeof(HEADER)]{};

    auto& registry = Registry::instance();

    if (reader.read(header, sizeof(HEADER))) [[unlikely]]
    {
        goto unexpectedEof;
    }

    if (memcmp(header, HEADER, sizeof(HEADER))) [[unlikely]]
    {
        goto incorrectHeader;
    }

    for (const auto& entry : registry.getDataEntries())
    {
        if (reader.read(entry.data, entry.size)) [[unlikely]]
        {
            goto unexpectedEof;
        }
    }

    for (auto event : registry.getEvents())
    {
        uint64_t when = 0;
        if (reader.read(&when, sizeof(when)))
        {
            goto unexpectedEof;
        }
        if (when)
        {
            gb.events.scheduleEvent(*event, when);
        }
    }

    return {};

unexpectedEof:
    return std::unexpected("Data too short");
incorrectHeader:
    return std::unexpected("Incorrect header");
}
