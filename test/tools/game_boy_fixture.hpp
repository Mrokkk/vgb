#pragma once

#include "test/tools/base_fixture.hpp"

namespace test::tools
{

struct GameBoyFixture : tools::BaseFixture
{
    void runRom(const void* data, size_t size);
    bool printAllInstructions = false;
private:
    void run();
};

}  // namespace test::tools
