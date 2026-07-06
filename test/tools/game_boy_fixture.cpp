#include "game_boy_fixture.hpp"

#include <cstdlib>

#include <doctest.h>

#include "src/cpu/isa/decoder.hpp"
#include "src/game_boy.hpp"
#include "test/tools/printers.hpp"
#include "test/tools/watchdog.hpp"

namespace test::tools
{

void GameBoyFixture::runRom(const void* data, size_t size)
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

    withWatchdog(1.0f, [this]{ run(); });

    CAPTURE(gb.cpu.exc);
    REQUIRE_EQ(gb.cpu.exc.type, cpu::Exception::InfiniteLoop);
}

void GameBoyFixture::run()
{
    if (printAllInstructions)
    {
        auto disCtx = cpu::isa::DisassembleContext::create(gb.cpu, gb.cpu.pc);
        gb.cpu.state = cpu::SM83::State::Running;
        while (1)
        {
            disCtx.pc = gb.cpu.pc;

            cpu::isa::disassemble(disCtx);

            fmt::println("{:04x}: {}", gb.cpu.pc.get(), disCtx.disassembled);
            if (gb.cpu.step() != 0 or gb.cpu.state != cpu::SM83::State::Running)
            {
                break;
            }
        }
    }
    else
    {
        gb.run();
    }
}

}  // namespace test::tools
