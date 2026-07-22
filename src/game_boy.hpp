#pragma once

#include <cstdint>
#include <functional>

#include "component.hpp"
#include "config.hpp"
#include "cpu/sm83.hpp"
#include "event_system.hpp"
#include "fwd.hpp"
#include "memory/cartridge.hpp"
#include "sys/renderer.hpp"
#include "utils/immobile.hpp"
#include "utils/unique_ptr.hpp"

struct GameBoy final : utils::Immobile
{
    enum class State : uint8_t
    {
        Stopped,
        Running,
    };

    GameBoy();
    ~GameBoy();

    void load(Config config);

    void run();

    void start();
    void stop();
    void reset();
    void skipBootRom();
    void frame();

    using Callback = std::move_only_function<void()>;

    void saveRam();

    void registerComponent(Component::Type type, utils::UniquePtr<Component> component);
    void withStoppedState(Callback callback);

    State             state;
    unsigned          speedMultiplier;
    size_t            counter;
    size_t            frameNumber;
    cpu::SM83         cpu;
    memory::Cartridge cartridge;
    EventSystem       events;
    Callback          scheduledCallback;
    sys::Texture      lcd;

    ComponentPtr components[Component::Type::Last + 1];
    Config config;
    bool inputEnabled;
    void* debuggerData;
};

extern GameBoy gb;
