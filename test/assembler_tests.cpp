#define NO_EXCEPTIONS
#include "src/assembler/assembler.hpp"
#include "src/cpu/sm83.hpp"
#include "src/game_boy.hpp"
#include "test/tools/base_fixture.hpp"
#include "test/tools/game_boy_fixture.hpp"
#include "test/tools/printers.hpp"
#include "test/tools/test_framework.hpp"

namespace test
{

const std::string_view invalidCode[]{
    "abc",
    "ld $9",
    "$32",
    "9392",
    "ld bc, $999999",
    "ill",
    "prefix",
    "ld a, $100",
    "ld a, [-$20]",
    "ld a, [",
    "ld a, $",
    "ld a, $-",
    "ld a, -$",
    "ld a, %",
    "jr -$fff",
    "add *",
    "rst 0x37",
    "rst $40",
    "ld hl, bc + 0x8\n",
    "ld [bc+], a",
    "ld [bc-], a",
};

TEST_CASE_FIXTURE_P(tools::BaseFixture, "Assembler :: failures", invalidCode)
{
    fakePlatform.addFile("/test.asm", arg); \
    auto result = assembler::assemble("/test.asm");
    EXPECT_FALSE(result);
}

#define ASSEMBLE(CODE) \
    ({ \
        const char* code = \
            "SECTION \"Header\", ROM0[$100]\n" \
            "   nop\n" \
            "   jp $150\n" \
            "SECTION \"Start\", ROM0[$150]\n" \
            CODE; \
        fakePlatform.addFile("/test.asm", code); \
        auto res = assembler::assemble("/test.asm"); \
        res; \
    })

TEST_CASE_FIXTURE(tools::GameBoyFixture, "Assembler :: basics")
{
    auto result = ASSEMBLE(
        "ld bc, $1234 ; some comment\n"
        "ld a, [$ffd0]\n"
        "ld d, $f9\n"
        "ld hl, $ffd1\n"
        "ld l, [hl]\n"
        "set $0, a\n"
        "ldh [$d3], a\n"
        "jr -2 ; infinite loop\n");

    REQUIRE(result);

    loadRom(result->data(), result->size());

    gb.cpu.mem.store8(0xffd0, 0xd0);
    gb.cpu.mem.store8(0xffd1, 0x20);

    run();

    EXPECT_EQ(gb.cpu.bc, 0x1234);
    EXPECT_EQ(gb.cpu.a, 0xd1);
    EXPECT_EQ(gb.cpu.d, 0xf9);
    EXPECT_EQ(gb.cpu.l, 0x20);
    EXPECT_EQ(gb.cpu.mem.load8(0xffd3), 0xd1);
}

TEST_CASE_FIXTURE(tools::GameBoyFixture, "Assembler :: basics2")
{
    auto result = ASSEMBLE(
        "ld sp, 0xfffe\n"
        "ld hl, sp - 0x12\n"
        "ld a, 0xc4\n"
        "ld b, h\n"
        "ld c, l\n"
        "ld sp, 0xff80\n"
        "ld hl, sp + 0x8\n"
        "ld [hl+], a\n"
        "ld hl, sp + 0x20\n"
        "srl a\n"
        "ld [hl-], a\n"
        "loop:\n"
        "\tjr loop");

    REQUIRE(result);

    runRom(result->data(), result->size());

    EXPECT_EQ(gb.cpu.a, 0x62);
    EXPECT_EQ(gb.cpu.bc, 0xffec);
    EXPECT_EQ(gb.cpu.hl, 0xff9f);
    EXPECT_EQ(gb.cpu.mem.load8(0xff88), 0xc4);
    EXPECT_EQ(gb.cpu.mem.load8(0xffa0), 0x62);
}

TEST_CASE_FIXTURE(tools::GameBoyFixture, "Assembler :: labels")
{
    auto result = ASSEMBLE(
        "   jr label1\n"
        "   ld bc, $1234\n"
        "   ld a, $d3\n"
        "   ld d, $f9\n"
        "\n"
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
        "   jr label2");

    REQUIRE(result);

    runRom(result->data(), result->size());

    EXPECT_EQ(gb.cpu.bc, 0x3333);
    EXPECT_EQ(gb.cpu.a, 0x3d);
    EXPECT_EQ(gb.cpu.d, 0x02);
}

TEST_CASE_FIXTURE(tools::GameBoyFixture, "Assembler :: include")
{
    const char* code =
        "ld a, 0xff\n"
        "ld b, 0x8f\n"
        "swap b\n";

    fakePlatform.addFile("/include.asm", code);

    auto result = ASSEMBLE(
        "INCLUDE \"/include.asm\"\n"
        "loop:\n"
        "   jr loop");

    REQUIRE(result);

    runRom(result->data(), result->size());
    EXPECT_EQ(gb.cpu.a, 0xff);
    EXPECT_EQ(gb.cpu.b, 0xf8);
}

}  // namespace test
