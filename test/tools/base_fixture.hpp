#pragma once

#include "test/tools/platform_fake.hpp"

namespace test::tools
{

struct BaseFixture
{
    PlatformFake fakePlatform;
};

}  // namespace test::tools
