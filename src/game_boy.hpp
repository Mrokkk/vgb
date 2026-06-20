#pragma once

#include "component.hpp"
#include "config.hpp"
#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "fwd.hpp"
#include "input.hpp"
#include "memory/cartridge.hpp"
#include "renderer.hpp"
#include "utils/immobile.hpp"
#include "utils/unique_ptr.hpp"

struct GameBoy final : utils::Immobile
{
    GameBoy();
    ~GameBoy();

    void start(void* cartridge, void* ram, const Config& config);
    void run();
    bool stop();
    void reset();
    void skipBootRom();
    void frame();

    void saveState();

    void registerComponent(Component::Type type, utils::UniquePtr<Component> component);

    unsigned          speedMultiplier;
    size_t            frameNumber;
    cpu::SM83         cpu;
    memory::Cartridge cartridge;
    EventSystem       events;

    ComponentPtr components[Component::Type::Last + 1];
    RendererPtr renderer;
    InputPtr input;
    Config config;
    bool inputEnabled;
    void* debuggerData;
};

extern GameBoy gb;
