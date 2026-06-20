#define LOG_HEADER "GameBoy"
#include "game_boy.hpp"

#include <cstdint>
#include <utility>

#include <fmt/base.h>

#include "apu.hpp"
#include "component.hpp"
#include "config.hpp"
#include "joypad.hpp"
#include "logger.hpp"
#include "ppu.hpp"
#include "renderer.hpp"
#include "sys/system.hpp"
#include "timer.hpp"
#include "utils/unique_ptr.hpp"

struct DummyComponent final : Component
{
    void reset() override
    {
    }

    void store(uint16_t, uint8_t) override
    {
    }

    uint8_t load(uint16_t) const override
    {
        return 0xff;
    }
};

GameBoy::GameBoy()
    : state(State::Stopped)
    , resetScheduled(false)
    , speedMultiplier(1)
    , frameNumber(0)
    , inputEnabled(true)
{
    for (auto& component : components)
    {
        component = utils::makeUnique<DummyComponent>();
    }
}

GameBoy::~GameBoy() = default;

void GameBoy::load(void* rom, void* ram, const Config& config)
{
    cartridge.initialize(rom, ram);

    this->config = config;

    createRenderer(*this, config);
    createInput(*this, config);

    createPpu(*this, config);
    createJoypad(*this, config);
    createApu(*this, config);
    createTimer(*this, config);

    if (config.skipBootRom)
    {
        skipBootRom();
    }
}

void GameBoy::run()
{
    state = State::Running;
    while (cpu.step() == 0 and state == State::Running);
}

void GameBoy::start()
{
    state = State::Running;
}

void GameBoy::stop()
{
    state = State::Stopped;
    cpu.stop();
}

void GameBoy::reset()
{
    if (state == State::Running)
    {
        stop();
        resetScheduled = true;
        return;
    }
    events.reset();
    cpu.reset();
    cartridge.reset();
    for (auto& component : components)
    {
        component->reset();
    }
    if (config.skipBootRom)
    {
        skipBootRom();
    }
}

void GameBoy::skipBootRom()
{
    config.skipBootRom = true;

    cpu.af  = 0x01b0;
    cpu.bc  = 0x0013;
    cpu.de  = 0x00d8;
    cpu.hl  = 0x014d;
    cpu.sp  = 0xfffe;
    cpu.pc  = 0x100;
    cpu.ie  = 0x00;
    cpu.$if = 0x00;
    cpu.ime = 0x0;

    cpu.mem.store8(0xff50, 0x01); // Disable boot ROM
    cpu.mem.store8(0xff40, 0x91); // Enable video
    cpu.mem.store8(0xff41, 0x85);
}

void GameBoy::frame()
{
    // Renderer should be locked to 60 FPS which is used to synchronize
    // GameBoy emulation. Skipping frames increases the emulation speed

    if (resetScheduled) [[unlikely]]
    {
        resetScheduled = false;
        reset();
        start();
    }

    static uint64_t counter = 0;
    if ((counter++ % speedMultiplier) == 0)
    {
        ++frameNumber;
        sys::pingSupervision();
        gb.input->update();
        gb.renderer->render();
    }
}

void GameBoy::saveState()
{
    if (cartridge.getRam() and cartridge.isRamDirty())
    {
        auto res = sys::saveToFile(config.cartridgeRamPath.c_str(), cartridge.getRam(), cartridge.ramSize());

        if (not res)
        {
            fmt::println(stderr, "{}: cannot save file: {}", config.cartridgeRamPath, res.error());
            exit(EXIT_FAILURE);
        }

        cartridge.setRamNotDirty();

        logger.info().write("saved RAM to {}", config.cartridgeRamPath);
    }
}

void GameBoy::registerComponent(Component::Type type, utils::UniquePtr<Component> component)
{
    components[static_cast<int>(type)] = std::move(component);
}
