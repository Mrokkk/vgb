#pragma once

#include "test/tools/platform_fake.hpp"

namespace test::tools
{

struct BaseFixture
{
protected:
    PlatformFake fakePlatform;
};

}  // namespace test::tools
