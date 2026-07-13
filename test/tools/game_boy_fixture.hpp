#pragma once

#include "test/tools/base_fixture.hpp"

namespace test::tools
{

struct GameBoyFixture : tools::BaseFixture
{
    void loadRom(const void* data, size_t size);
    void runRom(const void* data, size_t size);
    void step();
    void run();

    bool printAllInstructions = false;

private:
    void runImpl();
};

}  // namespace test::tools
