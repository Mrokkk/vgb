#include "system.hpp"

#include <fmt/base.h>

#include "config.hpp"
#include "sys/platform.hpp"
#include "sys/posix.hpp"
#include "sys/raylib/input.hpp"
#include "sys/raylib/renderer.hpp"
#include "sys/renderer.hpp"
#include "sys/supervision.hpp"
#include "utils/unique_ptr.hpp"

namespace sys
{

namespace
{

struct DummyRenderer final : Renderer
{
    void render() override {}
    void drawPixel(uint8_t, uint8_t, uint16_t) override {}
    unsigned int renderMap(bool) override { return -1; }
};

struct DummyInput : Input
{
    void update() override {}
    void subscribeForGameBoyInput(GameBoyInputCallback) override {}
};

}  // namespace

void initialize(const Config& config)
{
#ifdef __unix__
    posix::initialize(config);
#else
#error "Unsupported platform";
#endif

    if (config.useSupervision)
    {
        initSupervision();
    }

    if (config.mode != Mode::Headless)
    {
        raylib::createRenderer(config);
        raylib::createInput();
    }
    else
    {
        platform.renderer = utils::makeUnique<DummyRenderer>();
        platform.input = utils::makeUnique<DummyInput>();
    }
}

}  // namespace sys
