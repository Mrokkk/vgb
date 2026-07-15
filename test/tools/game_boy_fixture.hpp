#pragma once

#include <cstdint>
#include <vector>

#include "src/utils/immobile.hpp"
#include "test/tools/base_fixture.hpp"

namespace test::tools
{

struct GameBoyFixture : tools::BaseFixture, utils::Immobile
{
    void loadRomAndRam(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
    void loadRom(const void* data, size_t size);
    void runRom(const void* data, size_t size);
    void step();
    void run();

    bool printAllInstructions = false;

private:
    void load();
    void runImpl();

    static bool initialized;
};

}  // namespace test::tools
