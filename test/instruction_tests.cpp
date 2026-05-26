#include <string>

#include <doctest.h>

#include "config.hpp"
#include "game_boy.hpp"
#include "ppu/video.hpp"
#include "sys/system.hpp"

GameBoy gb;

struct TestData
{
    const char*   name;
    const uint8_t rom[32 * 1024];
};

static const TestData data[] = {
    TestData{
        .name = "01-special.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/01-special.gb"
#endif
        }
    },
    TestData{
        .name = "02-interrupts.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/02-interrupts.gb"
#endif
        }
    },
    TestData{
        .name = "03-op sp,hl.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/03-op sp,hl.gb"
#endif
        }
    },
    TestData{
        .name = "04-op r,imm.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/04-op r,imm.gb"
#endif
        }
    },
    TestData{
        .name = "05-op rp.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/05-op rp.gb"
#endif
        }
    },
    TestData{
        .name = "06-ld r,r.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/06-ld r,r.gb"
#endif
        }
    },
    TestData{
        .name = "07-jr,jp,call,ret,rst.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"
#endif
        }
    },
    TestData{
        .name = "08-misc instrs.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/08-misc instrs.gb"
#endif
        }
    },
    TestData{
        .name = "09-op r,r.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/09-op r,r.gb"
#endif
        }
    },
    TestData{
        .name = "10-bit ops.gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/10-bit ops.gb"
#endif
        }
    },
    TestData{
        .name = "11-op a,(hl).gb",
        .rom = {
#ifndef __clang__
#embed "test_roms/cpu_instrs/individual/11-op a,(hl).gb"
#endif
        }
    },
};

struct Fixture
{
    Fixture()
    {
        static bool initialized = false;
        static Config config{
            .skipBootRom = true,
            .videoConfig = VideoConfig::Headless,
        };
        if (not initialized)
        {
            sys::finalize();
            gb.vid.start(config);
            gb.cpu.skipBootRom();
            gb.cpu.timer.start();
            initialized = true;
        }
        else
        {
            gb.reset();
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

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

TEST_CASE_FIXTURE(Fixture, "instruction_tests")
{
    for (size_t i = 0; i < ARRAY_SIZE(data); ++i)
    {
        SUBCASE(data[i].name)
        {
            gb.cpu.mem.loadCartridge(data[i].rom);
            auto result = gb.cpu.run();
            CHECK_FALSE(result);
            auto str = readTestOutput();
            CHECK_FALSE(not str.contains("Passed"));
        }
    }
}
