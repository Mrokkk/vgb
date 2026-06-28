#define LOG_HEADER "SaveManager"
#include "save_manager.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>

#include <zlib.h>

#include "config.hpp"
#include "game_boy.hpp"
#include "logger.hpp"
#include "serializator.hpp"
#include "sys/system.hpp"
#include "utils/source_location.hpp"
#include "utils/units.hpp"

static std::filesystem::path saveDir;
static std::vector<Save> saves;
static constexpr auto quickSaveName = "quik.sav";

// TODO: (de)compress by chunks

static auto createSaveDirPath(const std::string& romFilePath)
{
    auto path = std::filesystem::path(romFilePath);
    path.replace_extension(".saves");
    return path;
}

static void reloadSaves()
{
    if (not sys::doesDirExist(saveDir.c_str()))
    {
        return;
    }

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(saveDir))
        {
            if (entry.is_regular_file())
            {
                saves.push_back({entry.path().filename().string()});
            }
        }
    }
    catch (const std::exception& e)
    {
        logger.error().write("error reading {} dir: {}", saveDir.native(), e.what());
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
    std::filesystem::path save(saveDir);
    save /= quickSaveName;

    std::filesystem::create_directories(saveDir);

    auto serialized = Serializator::serialize();

    if (not serialized) [[unlikely]]
    {
        logger.error().write("failed to save: {}", serialized.error());
        return;
    }

    Byte buffer[256 * KiB];
    size_t compressedSize = sizeof(buffer);

    if (auto error = compress(buffer, &compressedSize, (const Byte*)serialized->data(), serialized->size())) [[unlikely]]
    {
        logger.error().write("compression failed with {}", error);
        return;
    }

    auto result = sys::saveToFile(save.c_str(), buffer, compressedSize);

    if (not result) [[unlikely]]
    {
        logger.error().write("{}: failed to save: {}", save.native(), result.error());
        return;
    }

    logger.notice().write("saved {} with {} bytes (compressed from {})", save.native(), compressedSize, serialized->size());
}

void SaveManager::quickLoad()
{
    std::filesystem::path save(saveDir);
    save /= quickSaveName;

    auto mapped = sys::mapFile(save.c_str());

    if (not mapped) [[unlikely]]
    {
        logger.error().write("{}: failed to map: {}", save.native(), mapped.error());
        return;
    }

    std::vector<Byte> buffer(Serializator::getDataSize());
    size_t uncompressedSize = buffer.size();

    if (auto error = uncompress(buffer.data(), &uncompressedSize, static_cast<const Byte*>(mapped->getData()), mapped->getSize())) [[unlikely]]
    {
        logger.error().write("decompression failed: {}", error);
        return;
    }

    gb.withStoppedState(
        [buffer = std::move(buffer), uncompressedSize, save = std::move(save), func = __func__]
        {
            gb.events.reset();
            auto res = Serializator::deserialize(buffer.data(), uncompressedSize);

            if (not res) [[unlikely]]
            {
                logger.error(utils::SourceLocation::custom(func)).write("failed to deserialize: {}", res.error());
                return;
            }

            logger.notice(utils::SourceLocation::custom(func)).write("loaded {} with {} bytes", save.native(), uncompressedSize);
        });
}

const std::vector<Save>& SaveManager::getSaves()
{
    return saves;
}

const std::string& SaveManager::getSaveDir()
{
    return saveDir.native();
}
