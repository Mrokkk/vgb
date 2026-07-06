#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "src/config.hpp"
#include "src/game_boy.hpp"
#include "src/sys/system.hpp"
#include "test/tools/watchdog.hpp"

GameBoy gb;

int main(int argc, char** argv)
{
    gb.config = {
        .skipBootRom = true,
        .useSupervision = false,
        .mode = Mode::Headless,
    };

    test::tools::createWatchdog();
    sys::initialize(gb.config);
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    const int res = context.run();

    return res;
}
