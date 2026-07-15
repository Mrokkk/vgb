#include <algorithm>

#include "src/sys/mapped_file.hpp"
#include "src/sys/platform.hpp"
#include "sys/path.hpp"
#include "test/tools/base_fixture.hpp"
#include "test/tools/test_framework.hpp"

using namespace std::literals::string_view_literals;

namespace test::tools
{

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can add file with all ancestor directories")
{
    fakePlatform.addFile("/some/dir/file");
    EXPECT_FALSE(sys::readFileInfo("/somedir"));
    EXPECT_FALSE(sys::readFileInfo("/some/dir2"));
    EXPECT_FALSE(sys::readFileInfo("/some/dir/another_file"));
    {
        auto res = sys::readFileInfo("/some");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::Directory);
    }
    {
        auto res = sys::readFileInfo("/some/dir");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::Directory);
    }
    {
        auto res = sys::readFileInfo("/some/dir/file");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
        EXPECT_EQ(res->size, 0);
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can add file with content")
{
    char content[] = "Some content";
    fakePlatform.addFile("/somefile", content, sizeof(content));
    {
        auto res = sys::readFileInfo("/somefile");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
        EXPECT_EQ(res->size, sizeof(content));
    }
    {
        auto res = sys::mapFile("/somefile");
        REQUIRE(res);
        EXPECT_EQ(res->getSize(), sizeof(content));
        EXPECT_STREQ((const char*)res->getData(), content);
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can read file using relative path")
{
    fakePlatform.addFile("/dir/dir2/dir3/file");
    fakePlatform.setWorkingDirectory("/dir");
    EXPECT_FALSE(sys::readFileInfo("dir3"));
    EXPECT_FALSE(sys::readFileInfo("file"));
    {
        auto res = sys::readFileInfo("dir2/dir3/file");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
    }
    fakePlatform.setWorkingDirectory("dir2");
    EXPECT_FALSE(sys::readFileInfo("dir"));
    EXPECT_FALSE(sys::readFileInfo("file"));
    {
        auto res = sys::readFileInfo("dir3/file");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
    }
    fakePlatform.setWorkingDirectory("dir3");
    EXPECT_FALSE(sys::readFileInfo("dir2"));
    {
        auto res = sys::readFileInfo("file");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: throws on adding same file second time")
{
    fakePlatform.addFile("/somefile");
    EXPECT_THROWS(fakePlatform.addFile("/somefile"));
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: throws on trying to lookup relative path without working directory set")
{
    fakePlatform.addFile("/somefile");
    EXPECT_THROWS(({ auto res = sys::readFileInfo("somefile"); }));
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can write to file")
{
    fakePlatform.addDirectory("/somedir/nextdir");
    char content[] = "Some content";
    auto res = sys::writeToFile("/somedir/nextdir/file", content, sizeof(content));
    REQUIRE(res);
    {
        auto res = sys::readFileInfo("/somedir/nextdir/file");
        REQUIRE(res);
        EXPECT_EQ(res->type, sys::FileType::File);
        EXPECT_EQ(res->size, sizeof(content));
    }
    {
        auto res = sys::mapFile("/somedir/nextdir/file");
        REQUIRE(res);
        EXPECT_EQ(res->getData<const char>(), std::string_view(content));
        EXPECT_EQ(res->getSize(), sizeof(content));
    }
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can read empty directory")
{
    fakePlatform.addDirectory("/dir");
    sys::Path path("/dir");
    auto dirEntries = path.readDirectoryList();
    REQUIRE_EQ(dirEntries.size(), 0);
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can read directory")
{
    fakePlatform.addFile("/dir/file1");
    fakePlatform.addFile("/dir/file2");
    fakePlatform.addDirectory("/dir/dir1");
    fakePlatform.addDirectory("/dir/dir2");
    sys::Path path("/dir");
    auto dirEntries = path.readDirectoryList();
    std::sort(
        dirEntries.begin(), dirEntries.end(),
        [](const sys::DirEntry& e1, const sys::DirEntry& e2)
        {
            return std::string_view(e1.name) < std::string_view(e2.name);
        });
    REQUIRE_EQ(dirEntries.size(), 4);
    EXPECT_EQ(dirEntries[0].name, "dir1"sv);
    EXPECT_EQ(dirEntries[1].name, "dir2"sv);
    EXPECT_EQ(dirEntries[2].name, "file1"sv);
    EXPECT_EQ(dirEntries[3].name, "file2"sv);
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can try to read file as directory")
{
    fakePlatform.addFile("/file");
    sys::Path path("/file");
    auto dirEntries = path.readDirectoryList();
    REQUIRE_EQ(dirEntries.size(), 0);
}

TEST_CASE_FIXTURE(tools::BaseFixture, "PlatformFake :: can try to read non-existent directory")
{
    sys::Path path("/nonexistent-dir");
    auto dirEntries = path.readDirectoryList();
    REQUIRE_EQ(dirEntries.size(), 0);
}

}  // namespace test::tools
