#include "game_boy_fixture.hpp"

#include <cstdlib>

#include "src/cpu/isa/decoder.hpp"
#include "src/game_boy.hpp"
#include "test/tools/watchdog.hpp"

namespace test::tools
{

bool GameBoyFixture::textureDirty = false;
bool GameBoyFixture::initialized = false;

GameBoyFixture::GameBoyFixture()
{
    gb.lcd.dirty = &textureDirty;
}

void GameBoyFixture::loadRomAndRam(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
{
    fakePlatform.addFile("/test.rom", rom.data(), rom.size());
    fakePlatform.addFile("/test.ram", ram.data(), ram.size());
    fakePlatform.setWorkingDirectory("/");

    gb.config.cartridgePath = "test.rom";
    gb.config.cartridgeRamPath = "test.ram";

    load();
}

void GameBoyFixture::loadRom(const void* data, size_t size)
{
    fakePlatform.addFile("/test.rom", const_cast<void*>(data), size);
    fakePlatform.setWorkingDirectory("/");

    gb.config.cartridgePath = "test.rom";

    load();
}

void GameBoyFixture::runRom(const void* data, size_t size)
{
    loadRom(data, size);
    run();
}

void GameBoyFixture::run()
{
#ifdef __SANITIZE_ADDRESS__
    withWatchdog(10.0f, [this]{ runImpl(); });
#else
    withWatchdog(1.0f, [this]{ runImpl(); });
#endif
}

void GameBoyFixture::step()
{
    if (printAllInstructions)
    {
        auto disCtx = cpu::isa::DisassembleContext::create(gb.cpu, gb.cpu.pc);
        cpu::isa::disassemble(disCtx);

        fmt::println("{:04x}: {}", gb.cpu.pc.get(), disCtx.disassembled);
    }
    gb.cpu.step();
}

void GameBoyFixture::load()
{
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

void GameBoyFixture::runImpl()
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
