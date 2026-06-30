#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "config.hpp"
#include "sys/system.hpp"
#include "game_boy.hpp"

GameBoy gb;

int main(int argc, char** argv)
{
    gb.config = {
        .skipBootRom = true,
        .useSupervision = false,
        .videoConfig = VideoConfig::Headless,
    };

    sys::initialize(gb.config);
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    const int res = context.run();

    return res;
}
