#define LOG_HEADER "SaveManager"
#include "save_manager.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>

#include <zlib.h>

#include "config.hpp"
#include "core/logger.hpp"
#include "game_boy.hpp"
#include "save_serializer.hpp"
#include "sys/path.hpp"
#include "sys/platform.hpp"
#include "utils/source_location.hpp"
#include "utils/units.hpp"

using namespace utils::literals;

static sys::Path saveDir;
static std::vector<Save> saves;
static constexpr auto quickSaveName = "quik.sav";

// TODO: (de)compress by chunks

static auto createSaveDirPath(const std::string& romFilePath)
{
    auto path = sys::Path(romFilePath);
    path.replaceExtension(".saves");
    return path;
}

static void reloadSaves()
{
    if (not sys::isDirectory(saveDir.cString()))
    {
        return;
    }

    try
    {
        for (const auto& entry : saveDir.readDirectory())
        {
            if (entry.info.type == sys::FileType::File)
            {
                saves.push_back({entry.name});
            }
        }
    }
    catch (const std::exception& e)
    {
        core::logger.error().write("error reading {} dir: {}", saveDir.string(), e.what());
    }
}

void SaveManager::init(const Config& config)
{
    saves.clear();
    saveDir = createSaveDirPath(config.cartridgePath);

    reloadSaves();
}

void SaveManager::quickSave()
{
    sys::Path save(saveDir);
    save /= quickSaveName;

    if (auto res = sys::createDirectory(saveDir.cString()); not res) [[unlikely]]
    {
        core::logger.error().write("failed to create save dir: {}", res.error());
        return;
    }

    auto serialized = SaveSerializer::serialize();

    if (not serialized) [[unlikely]]
    {
        core::logger.error().write("failed to save: {}", serialized.error());
        return;
    }

    Byte buffer[256_KiB];
    size_t compressedSize = sizeof(buffer);

    if (auto error = compress(buffer, &compressedSize, (const Byte*)serialized->data(), serialized->size())) [[unlikely]]
    {
        core::logger.error().write("compression failed with {}", error);
        return;
    }

    auto result = sys::writeToFile(save.cString(), buffer, compressedSize);

    if (not result) [[unlikely]]
    {
        core::logger.error().write("{}: failed to save: {}", save.string(), result.error());
        return;
    }

    core::logger.notice().write("saved {} with {} bytes (compressed from {})", save.string(), compressedSize, serialized->size());
}

void SaveManager::quickLoad()
{
    sys::Path save(saveDir);
    save /= quickSaveName;

    auto mapped = sys::mapFile(save.cString());

    if (not mapped) [[unlikely]]
    {
        core::logger.error().write("{}: failed to map: {}", save.string(), mapped.error());
        return;
    }

    std::vector<Byte> buffer(SaveSerializer::getDataSize());
    size_t uncompressedSize = buffer.size();

    if (auto error = uncompress(buffer.data(), &uncompressedSize, static_cast<const Byte*>(mapped->getData()), mapped->getSize())) [[unlikely]]
    {
        core::logger.error().write("decompression failed: {}", error);
        return;
    }

    gb.withStoppedState(
        [buffer = std::move(buffer), uncompressedSize, save = std::move(save), func = __func__]
        {
            gb.events.reset(); // FIXME: restore on failure
            auto res = SaveSerializer::deserialize(buffer.data(), uncompressedSize);

            if (not res) [[unlikely]]
            {
                core::logger.error(utils::SourceLocation::custom(func)).write("failed to deserialize: {}", res.error());
                return;
            }

            core::logger.notice(utils::SourceLocation::custom(func)).write("loaded {} with {} bytes", save.string(), uncompressedSize);
        });
}

const std::vector<Save>& SaveManager::getSaves()
{
    return saves;
}

const std::string& SaveManager::getSaveDir()
{
    return saveDir.string();
}
