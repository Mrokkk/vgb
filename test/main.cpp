#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "config.hpp"
#include "sys/system.hpp"
#include "test_config.hpp"

Config testConfig = {
    .skipBootRom = true,
    .useSupervision = false,
    .videoConfig = VideoConfig::Headless,
};

int main(int argc, char** argv)
{
    sys::initialize(testConfig);
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    const int res = context.run();

    return res;
}
