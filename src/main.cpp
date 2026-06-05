#include <argh.h>
#include <fmt/base.h>

#include "debugger/main.hpp"
#include "config.hpp"
#include "game_boy.hpp"
#include "sys/system.hpp"

GameBoy gb;

std::string createRamFilePath(const std::string& romFilePath)
{
    auto dot = romFilePath.find_last_of('.', romFilePath.size());
    std::string ramFilePath(romFilePath.begin(), romFilePath.begin() + dot);
    ramFilePath += ".vgb";
    return ramFilePath;
}

int main(int argc, char* argv[])
{
    sys::initialize();
    argh::parser cmdl(argc, argv);

    if (cmdl.size() != 2)
    {
        fmt::println(stderr, "Expected one positional argument, got {}", cmdl.size() - 1);
        sys::finalize();
        return EXIT_FAILURE;
    }

    const Config config{
        .cartridgePath = cmdl[1],
        .cartridgeRamPath = createRamFilePath(cmdl[1]),
        .skipBootRom = cmdl[{"-f", "--skip-boot"}],
        .useDebugger = cmdl[{"-g", "--debugger"}],
    };

    const auto mappedRom = sys::mapFile(config.cartridgePath.c_str());

    if (not mappedRom) [[unlikely]]
    {
        fmt::println(stderr, "{}: cannot map: {}", config.cartridgePath, mappedRom.error());
        sys::finalize();
        return EXIT_FAILURE;
    }

    sys::MappedFile mappedRam;

    if (sys::doesFileExist(config.cartridgeRamPath.c_str()))
    {
        auto result = sys::mapFile(config.cartridgeRamPath.c_str(), false);

        if (not result) [[unlikely]]
        {
            fmt::println(stderr, "{}: cannot map: {}", config.cartridgeRamPath, result.error());
            sys::finalize();
            return EXIT_FAILURE;
        }

        mappedRam = *result;
    }
    else
    {
        mappedRam.ptr = nullptr;
    }

    gb.start(mappedRom->ptr, mappedRam.ptr, config);

    if (config.useDebugger)
    {
        debugger::main();
    }
    else
    {
        gb.run();
    }

    sys::finalize();

    if (gb.cartridge.getRam())
    {
        auto res = sys::saveToFile(config.cartridgeRamPath.c_str(), gb.cartridge.getRam(), gb.cartridge.ramSize());

        if (not res)
        {
            fmt::println(stderr, "{}: cannot save file: {}", config.cartridgeRamPath, res.error());
            return EXIT_FAILURE;
        }
    }

    return 0;
}
