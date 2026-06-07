#include "input.hpp"

#include "config.hpp"
#include "game_boy.hpp"
#include "raylib/input.hpp"
#include "utils/unique_ptr.hpp"

struct DummyInput : Input
{
    void update() override {}
    void subscribeForGameBoyInput(GameBoyInputCallback) override {}
};

void createInput(GameBoy& gb, const Config& config)
{
    if (config.videoConfig == VideoConfig::Graphical)
    {
        raylib::createInput(gb, config);
    }
    else
    {
        gb.input = utils::makeUnique<DummyInput>();
    }
}
