#include <string>

#include <doctest.h>

#include "src/game_boy.hpp"
#include "src/utils/units.hpp"
#include "test/tools/compiler.hpp"
#include "test/tools/game_boy_fixture.hpp"
#include "test/tools/printers.hpp"

namespace test
{

namespace
{

struct TestData final
{
    const char*   name;
    const uint8_t rom[32 * KiB];
};

CLANG_DIAGNOSTIC_PUSH()
CLANG_DIAGNOSTIC_IGNORED("-Wc23-extensions")
const TestData data[] = {
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

struct Fixture : tools::GameBoyFixture
{
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

}  // namespace

TEST_CASE_FIXTURE(Fixture, "Test ROMs")
{
    for (size_t i = 0; i < ARRAY_SIZE(data); ++i)
    {
        SUBCASE(data[i].name)
        {
            runRom(data[i].rom, sizeof(data[i]));
            auto testOutput = readTestOutput();
            if (not testOutput.contains("Passed")) [[unlikely]]
            {
                FAIL_CHECK("Failed test; output:\n", testOutput);
            }
        }
    }
}

}  // namespace test
