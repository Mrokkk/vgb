#include "renderer.hpp"

#include "config.hpp"
#include "game_boy.hpp"
#include "raylib/renderer.hpp"
#include "utils/unique_ptr.hpp"

struct DummyRenderer final : Renderer
{
    void render() override {}
    void drawPixel(uint8_t, uint8_t, uint16_t) override {}
    unsigned int renderMap(bool) override { return -1; }
};

void createRenderer(GameBoy& gb, const Config& config)
{
    if (config.videoConfig == VideoConfig::Graphical)
    {
        raylib::createRenderer(gb);
    }
    else
    {
        gb.renderer = utils::makeUnique<DummyRenderer>();
    }
}
