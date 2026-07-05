#include <cstdlib>

#include <argh.h>
#include <fmt/base.h>

#include "config.hpp"
#include "debugger/main.hpp"
#include "game_boy.hpp"
#include "sys/path.hpp"
#include "sys/system.hpp"

GameBoy gb;

static std::string createRamFilePath(const std::string& romFilePath)
{
    auto path = sys::Path(romFilePath);
    path.replaceExtension(".vgb");
    return path.release();
}

int main(int argc, char* argv[])
{
    const argh::parser cmdl(argc, argv);

    if (cmdl.size() != 2) [[unlikely]]
    {
        fmt::println(stderr, "Expected one positional argument, got {}", cmdl.size() - 1);
        exit(EXIT_FAILURE);
    }

    Config config{
        .cartridgePath = cmdl[1],
        .cartridgeRamPath = createRamFilePath(cmdl[1]),
        .skipBootRom = cmdl[{"-f", "--skip-boot"}],
        .useSupervision = true,
        .mode = cmdl[{"-m", "--minimal"}] ? Mode::Minimal : Mode::Debugger
    };

    sys::initialize(config);
    gb.load(config);

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
