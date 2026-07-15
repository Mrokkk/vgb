#include "src/config.hpp"
#include "src/game_boy.hpp"
#include "src/sys/system.hpp"
#include "test/tools/test_framework.hpp"
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

    tools::getConfig().printIntegersAsHex = true;

    return TEST_CASES_RUN(argc, argv);
}
