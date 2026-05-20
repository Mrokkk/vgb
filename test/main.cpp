#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "sys/system.hpp"

int main(int argc, char** argv)
{
    sys::initialize();
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    const int res = context.run();

    sys::finalize();

    return res;
}
