#define NO_EXCEPTIONS
#include <string>

#include "src/game_boy.hpp"
#include "src/utils/units.hpp"
#include "test/tools/compiler.hpp"
#include "test/tools/game_boy_fixture.hpp"
#include "test/tools/test_framework.hpp"

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

}  // namespace

template <>
struct TestStringConverter<TestData>
{
    static std::string convert(const TestData& t)
    {
        return t.name;
    }
};

namespace test
{

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

TEST_CASE_FIXTURE_P(Fixture, "Test ROMs", data)
{
    runRom(arg.rom, sizeof(arg.rom));
    auto testOutput = readTestOutput();
    if (not testOutput.contains("Passed")) [[unlikely]]
    {
        FAIL("Failed test; output:\n%s", testOutput.c_str());
    }
}

}  // namespace test
