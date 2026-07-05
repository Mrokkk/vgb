#include <doctest.h>

#include "src/sys/path.hpp"
#include "test/tools/base_fixture.hpp"

using sys::Path;

namespace test::sys
{

TEST_CASE_FIXTURE(tools::BaseFixture, "Path")
{
    SUBCASE("can be created empty")
    {
        Path path;
        REQUIRE_FALSE(path);
    }

    SUBCASE("can be created with cstring")
    {
        Path path("/some/path");
        REQUIRE(path);
    }

    SUBCASE("can return string")
    {
        Path path("/some/path");
        REQUIRE(path);
        CHECK_EQ(path.string(), "/some/path");
    }

    SUBCASE("can return cstring")
    {
        Path path("/some/path");
        REQUIRE(path);
        CHECK_EQ(path.cString(), std::string_view("/some/path"));
    }

    SUBCASE("can release string")
    {
        Path path("/some/path");
        REQUIRE(path);
        auto string = path.release();
        CHECK_FALSE(path);
        CHECK_EQ(string, "/some/path");
    }

    SUBCASE("can get extension")
    {
        Path path("/some/path/file_without_ext");
        auto ext = path.extension();
        CHECK_EQ(ext.size(), 0);
        CHECK_EQ(path.extension(), std::string_view(""));
    }

    SUBCASE("can add extension")
    {
        Path path("/some/path/file_without_ext");
        path.replaceExtension(".ext");
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path/file_without_ext.ext");
    }

    SUBCASE("can replace extension with shorter one")
    {
        Path path("/some/path/file_with_ext.test");
        path.replaceExtension(".ext");
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path/file_with_ext.ext");
    }

    SUBCASE("can replace extension with longer one")
    {
        Path path("/some/path/file_with_ext.test");
        path.replaceExtension(".long_ext");
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path/file_with_ext.long_ext");
    }

    SUBCASE("can add next path part")
    {
        Path path("/some/path");
        path /= "file";
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path/file");
    }

    SUBCASE("can add separators and path will not change")
    {
        Path path("/some/path");
        path /= "////";
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path");
    }

    SUBCASE("can add next path part with; additional / will be removed")
    {
        Path path("/some/path");
        path /= "//file//";
        CHECK(path);
        CHECK_EQ(path.string(), "/some/path/file");
    }

    SUBCASE("can get dirname of absolute path")
    {
        {
            Path path("/some/path");
            CHECK_EQ(path.dirname(), "/some");
        }
        {
            Path path("/");
            CHECK_EQ(path.dirname(), "/");
        }
    }

    SUBCASE("can get dirname of relative path")
    {
        {
            Path path("some/path");
            CHECK_EQ(path.dirname(), "some");
        }
        {
            Path path("path");
            CHECK_EQ(path.dirname(), ".");
        }
    }

    SUBCASE("can get basename of absolute path")
    {
        {
            Path path("/some/path");
            CHECK_EQ(path.basename(), "path");
        }
        {
            Path path("/");
            CHECK_EQ(path.basename(), "/");
        }
    }

    SUBCASE("can get basename of relative path")
    {
        {
            Path path("some/path");
            CHECK_EQ(path.basename(), "path");
        }
        {
            Path path("some");
            CHECK_EQ(path.basename(), "some");
        }
    }
}

}  // namespace sys::tests
