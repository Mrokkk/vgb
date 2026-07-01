#include <cstdlib>

#include <argh.h>
#include <filesystem>
#include <fmt/base.h>

#include "config.hpp"
#include "debugger/main.hpp"
#include "game_boy.hpp"
#include "sys/platform.hpp"
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

    gb.config = {
        .cartridgePath = cmdl[1],
        .cartridgeRamPath = createRamFilePath(cmdl[1]),
        .skipBootRom = cmdl[{"-f", "--skip-boot"}],
        .useSupervision = true,
        .mode = cmdl[{"-g", "--debugger"}] ? Mode::Debugger : Mode::Minimal
    };

    sys::initialize(gb.config);

    const auto mappedRom = sys::mapFile(gb.config.cartridgePath.c_str(), false);

    if (not mappedRom) [[unlikely]]
    {
        fmt::println(stderr, "{}: cannot map: {}", gb.config.cartridgePath, mappedRom.error());
        return EXIT_FAILURE;
    }

    sys::MappedFile mappedRam;

    if (sys::doesFileExist(gb.config.cartridgeRamPath.c_str()))
    {
        auto result = sys::mapFile(gb.config.cartridgeRamPath.c_str(), false);

        if (not result) [[unlikely]]
        {
            fmt::println(stderr, "{}: cannot map: {}", gb.config.cartridgeRamPath, result.error());
            return EXIT_FAILURE;
        }

        mappedRam = std::move(*result);
    }

    gb.load(mappedRom->getData(), mappedRam.getData(), gb.config);

    if (gb.config.mode == Mode::Debugger)
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
