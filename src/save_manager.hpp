#pragma once

#include <string>
#include <vector>

#include "fwd.hpp"

struct Save final
{
    std::string name;
};

struct SaveManager final
{
    static void init(const Config& config);

    static void quickSave();
    static void quickLoad();

    static const std::vector<Save>& getSaves();
    static const std::string& getSaveDir();
};
