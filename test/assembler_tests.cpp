#include <doctest.h>

#include "cpu/isa/assembler.hpp"
#include "src/cpu/exception.hpp"
#include "src/game_boy.hpp"
#include "test/tools/base_fixture.hpp"
#include "test/tools/game_boy_fixture.hpp"
#include "test/tools/printers.hpp"

namespace test
{

TEST_CASE_FIXTURE(tools::BaseFixture, "Assembler failures")
{
    const std::vector<std::string_view> invalidCode{
        "abc",
        "ld $9",
        "$32",
        "9392",
        "ld bc, $999999",
        "ill",
        "prefix",
        "ld a, $100",
        "ld a, [$-20]",
        "ld a, [",
        "ld a, $",
        "ld a, $-",
        "jr $-fff",
        "add *",
        "rst$40",
    };
    for (const auto& code : invalidCode)
    {
        SUBCASE(code.data())
        {
            CAPTURE(code);
            auto result = cpu::isa::assemble(code);
            CHECK_FALSE(result);
        }
    }
}

#define ASSEMBLE(CODE) \
    cpu::isa::assemble( \
        "SECTION \"Header\", ROM0[$100]\n" \
        "   nop\n" \
        "   jp $150\n" \
        "SECTION \"Start\", ROM0[$150]\n" \
        CODE)

TEST_CASE_FIXTURE(tools::GameBoyFixture, "Assembler")
{
    SUBCASE("basics")
    {
        gb.cpu.mem.store8(0xffd0, 0xd0);
        gb.cpu.mem.store8(0xffd1, 0x20);

        auto result = ASSEMBLE(
            "ld bc, $1234 ; some comment\n"
            "ld a, [$ffd0]\n"
            "ld d, $f9\n"
            "ld hl, $ffd1\n"
            "ld l, [hl]\n"
            "set $0, a\n"
            "ldh [$d3], a\n"
            "jr $-2 ; infinite loop");

        REQUIRE(result);

        runRom(result->data(), result->size());

        CHECK_EQ(gb.cpu.bc, 0x1234);
        CHECK_EQ(gb.cpu.a, 0xd1);
        CHECK_EQ(gb.cpu.d, 0xf9);
        CHECK_EQ(gb.cpu.l, 0x20);
        CHECK_EQ(gb.cpu.mem.load8(0xffd3), 0xd1);
        CHECK_EQ(gb.cpu.exc.type, cpu::Exception::InfiniteLoop);
    }

    SUBCASE("labels")
    {
        auto result = ASSEMBLE(
            "   jr label1\n"
            "   ld bc, $1234\n"
            "   ld a, $d3\n"
            "   ld d, $f9\n"
            "label1:\n"
            "   ld bc, $3333\n"
            "   ld a, $3c\n"
            "   ld d, $02\n"
            "   cp a, $03\n"
            "   jp nz, label3\n"
            "label2:\n"
            "   jr label2\n"
            "label3:\n"
            "   inc a\n"
            "   jr label2\n");

        REQUIRE(result);

        runRom(result->data(), result->size());

        CHECK_EQ(gb.cpu.bc, 0x3333);
        CHECK_EQ(gb.cpu.a, 0x3d);
        CHECK_EQ(gb.cpu.d, 0x02);
    }
}

}  // namespace test
