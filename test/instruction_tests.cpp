#include <string>

#include <doctest.h>

#include "src/cpu/exception.hpp"
#include "src/game_boy.hpp"
#include "src/utils/units.hpp"
#include "test/tools/base_fixture.hpp"
#include "test/tools/compiler.hpp"

namespace test
{

struct TestData final
{
    const char*   name;
    const uint8_t rom[32 * KiB];
};

CLANG_DIAGNOSTIC_PUSH()
CLANG_DIAGNOSTIC_IGNORED("-Wc23-extensions")
static const TestData data[] = {
    TestData{
        .name = "01-special.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/01-special.gb"
        }
    },
    TestData{
        .name = "02-interrupts.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/02-interrupts.gb"
        }
    },
    TestData{
        .name = "03-op sp,hl.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/03-op sp,hl.gb"
        }
    },
    TestData{
        .name = "04-op r,imm.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/04-op r,imm.gb"
        }
    },
    TestData{
        .name = "05-op rp.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/05-op rp.gb"
        }
    },
    TestData{
        .name = "06-ld r,r.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/06-ld r,r.gb"
        }
    },
    TestData{
        .name = "07-jr,jp,call,ret,rst.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"
        }
    },
    TestData{
        .name = "08-misc instrs.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/08-misc instrs.gb"
        }
    },
    TestData{
        .name = "09-op r,r.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/09-op r,r.gb"
        }
    },
    TestData{
        .name = "10-bit ops.gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/10-bit ops.gb"
        }
    },
    TestData{
        .name = "11-op a,(hl).gb",
        .rom = {
#embed "test_roms/cpu_instrs/individual/11-op a,(hl).gb"
        }
    },
};
CLANG_DIAGNOSTIC_POP()

struct Fixture : tools::BaseFixture
{
    void loadRom(const void* data, size_t size)
    {
        static bool initialized = false;

        fakePlatform.addFile("/test.rom", const_cast<void*>(data), size);
        fakePlatform.setWorkingDirectory("/");
        gb.config.cartridgePath = "test.rom";

        if (not initialized)
        {
            gb.load(gb.config);
            initialized = true;
        }
        else
        {
            gb.stop();
            gb.reset();
            gb.cartridge.initialize(gb.config);
        }
    }

    std::string readTestOutput() const
    {
        char data[256];
        size_t index = 0;
        for (size_t i = 0; i < sizeof(data); ++i)
        {
            auto val = gb.cpu.mem.load8(0x9800 + i);
            if (index > 0)
            {
                if (val == ' ' and (data[index - 1] == ' ' or data[index - 1] == '\n'))
                {
                    data[index - 1] = '\n';
                    continue;
                }
            }
            data[index++] = val;
        }
        data[index++] = 0;
        return std::string(data);
    }
};

TEST_CASE_FIXTURE(Fixture, "Instructions")
{
    for (size_t i = 0; i < ARRAY_SIZE(data); ++i)
    {
        SUBCASE(data[i].name)
        {
            loadRom(data[i].rom, sizeof(data[i]));
            gb.run();
            CHECK_EQ(gb.cpu.exc.type, cpu::Exception::InfiniteLoop);
            auto str = readTestOutput();
            CHECK(str.contains("Passed"));
        }
    }
}

}  // namespace test
