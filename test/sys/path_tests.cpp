#define NO_EXCEPTIONS
#include "src/sys/path.hpp"
#include "test/tools/base_fixture.hpp"
#include "test/tools/test_framework.hpp"

using sys::Path;

namespace test::sys
{

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can be created empty")
{
    Path path;
    REQUIRE_FALSE(path);
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can be created with cstring")
{
    Path path("/some/path");
    REQUIRE(path);
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can return string")
{
    Path path("/some/path");
    REQUIRE(path);
    EXPECT_EQ(path.string(), "/some/path");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can return cstring")
{
    Path path("/some/path");
    REQUIRE(path);
    EXPECT_EQ(path.cString(), std::string_view("/some/path"));
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can release string")
{
    Path path("/some/path");
    REQUIRE(path);
    auto string = path.release();
    EXPECT_FALSE(path);
    EXPECT_EQ(string, "/some/path");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can get extension")
{
    Path path("/some/path/file_without_ext");
    auto ext = path.extension();
    EXPECT_EQ(ext.size(), 0);
    EXPECT_EQ(path.extension(), std::string_view(""));
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can add extension")
{
    Path path("/some/path/file_without_ext");
    path.replaceExtension(".ext");
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path/file_without_ext.ext");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can replace extension with shorter one")
{
    Path path("/some/path/file_with_ext.test");
    path.replaceExtension(".ext");
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path/file_with_ext.ext");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can replace extension with longer one")
{
    Path path("/some/path/file_with_ext.test");
    path.replaceExtension(".long_ext");
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path/file_with_ext.long_ext");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can add next path part")
{
    Path path("/some/path");
    path /= "file";
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path/file");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can add separators and path will not change")
{
    Path path("/some/path");
    path /= "////";
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can add next path part with; additional / will be removed")
{
    Path path("/some/path");
    path /= "//file//";
    EXPECT(path);
    EXPECT_EQ(path.string(), "/some/path/file");
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can get dirname of absolute path")
{
    {
        Path path("/some/path");
        EXPECT_EQ(path.dirname(), "/some");
    }
    {
        Path path("/");
        EXPECT_EQ(path.dirname(), "/");
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can get dirname of relative path")
{
    {
        Path path("some/path");
        EXPECT_EQ(path.dirname(), "some");
    }
    {
        Path path("path");
        EXPECT_EQ(path.dirname(), ".");
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can get basename of absolute path")
{
    {
        Path path("/some/path");
        EXPECT_EQ(path.basename(), "path");
    }
    {
        Path path("/");
        EXPECT_EQ(path.basename(), "/");
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "Path :: can get basename of relative path")
{
    {
        Path path("some/path");
        EXPECT_EQ(path.basename(), "path");
    }
    {
        Path path("some");
        EXPECT_EQ(path.basename(), "some");
    }
}

}  // namespace sys::tests
