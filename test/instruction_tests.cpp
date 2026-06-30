#include <string>

#include <doctest.h>

#include "apu.hpp"
#include "cpu/exception.hpp"
#include "game_boy.hpp"
#include "joypad.hpp"
#include "ppu.hpp"
#include "timer.hpp"
#include "utils/units.hpp"

struct TestData
{
    const char*   name;
    const uint8_t rom[32 * KiB];
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
        if (not initialized)
        {
            createPpu(gb);
            createJoypad(gb);
            createApu(gb);
            createTimer(gb);
            gb.skipBootRom();
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
            gb.cartridge.initialize(const_cast<uint8_t*>(data[i].rom), nullptr);
            gb.run();
            CHECK_EQ(gb.cpu.exc.type, cpu::Exception::InfiniteLoop);
            auto str = readTestOutput();
            CHECK_FALSE(not str.contains("Passed"));
        }
    }
}
