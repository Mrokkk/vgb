#include <argh.h>
#include <fmt/base.h>

#include "debugger/main.hpp"
#include "config.hpp"
#include "game_boy.hpp"
#include "sys/system.hpp"

GameBoy gb;

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
        .skipBootRom = cmdl[{"-f", "--skip-boot"}],
        .useDebugger = cmdl[{"-g", "--debugger"}],
    };

    auto mappedRom = sys::mapFile(config.cartridgePath.c_str());

    if (not mappedRom) [[unlikely]]
    {
        fmt::println(stderr, "{}: cannot map: {}", argv[1], mappedRom.error());
        sys::finalize();
        return EXIT_FAILURE;
    }

    if (config.useDebugger)
    {
        debugger::main(mappedRom->ptr, config);
    }
    else
    {
        gb.run(mappedRom->ptr, config);
    }

    sys::finalize();

    return 0;
}
