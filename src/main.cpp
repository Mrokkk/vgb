#include <fmt/base.h>

#include "cpu/sm83.hpp"
#include "debugger/main.hpp"
#include "game_boy.hpp"
#include "memory/memory.hpp"
#include "sys/system.hpp"

GameBoy gb;

int main(int argc, char* argv[])
{
    sys::initialize();

    if (argc < 2)
    {
        fmt::println(stderr, "Incorrect arguments");
        sys::finalize();
        return EXIT_FAILURE;
    }

    auto mappedRom = sys::mapFile(argv[1]);

    if (not mappedRom) [[unlikely]]
    {
        fmt::println(stderr, "Cannot map file: {}", mappedRom.error());
        sys::finalize();
        return EXIT_FAILURE;
    }

    gb.cpu.mem.loadCartridge(mappedRom->ptr);
    gb.vid.start();

    if (0)
    {
        gb.cpu.run();
    }
    else
    {
        debugger::main();
    }

    gb.vid.stop();

    sys::finalize();

    return 0;
}
