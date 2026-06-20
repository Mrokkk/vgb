#include <cstdlib>

#include <argh.h>
#include <filesystem>
#include <fmt/base.h>

#include "debugger/main.hpp"
#include "config.hpp"
#include "game_boy.hpp"
#include "sys/system.hpp"

GameBoy gb;

static std::string createRamFilePath(const std::string& romFilePath)
{
    auto path = std::filesystem::path(romFilePath);
    path.replace_extension(".vgb");
    return path.string();
}

int main(int argc, char* argv[])
{
    const argh::parser cmdl(argc, argv);

    if (cmdl.size() != 2)
    {
        fmt::println(stderr, "Expected one positional argument, got {}", cmdl.size() - 1);
        return EXIT_FAILURE;
    }

    const Config config{
        .cartridgePath = cmdl[1],
        .cartridgeRamPath = createRamFilePath(cmdl[1]),
        .skipBootRom = cmdl[{"-f", "--skip-boot"}],
        .useDebugger = cmdl[{"-g", "--debugger"}],
        .useSupervision = true,
        .videoConfig = VideoConfig::Graphical
    };

    sys::initialize(config);

    const auto mappedRom = sys::mapFile(config.cartridgePath.c_str(), false);

    if (not mappedRom) [[unlikely]]
    {
        fmt::println(stderr, "{}: cannot map: {}", config.cartridgePath, mappedRom.error());
        return EXIT_FAILURE;
    }

    sys::MappedFile mappedRam;

    if (sys::doesFileExist(config.cartridgeRamPath.c_str()))
    {
        auto result = sys::mapFile(config.cartridgeRamPath.c_str(), false);

        if (not result) [[unlikely]]
        {
            fmt::println(stderr, "{}: cannot map: {}", config.cartridgeRamPath, result.error());
            return EXIT_FAILURE;
        }

        mappedRam = std::move(*result);
    }

    gb.load(mappedRom->getData(), mappedRam.getData(), config);

    if (config.useDebugger)
    {
        debugger::main(gb);
    }
    else
    {
        gb.run();
    }

    gb.saveRam();

    return 0;
}
