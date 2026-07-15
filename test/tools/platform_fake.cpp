#include "platform_fake.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <map>
#include <utility>
#include <vector>

#include "src/error.hpp"
#include "src/sys/platform.hpp"
#include "src/utils/memory.hpp"
#include "src/utils/string.hpp"
#include "src/utils/units.hpp"
#include "test/tools/assert.hpp"

using sys::DirEntry;
using sys::FileInfo;
using sys::FileType;
using sys::Mapped;
using sys::MaybeDirEntry;
using sys::MaybeError;
using sys::MaybeFileInfo;
using sys::MaybeMapped;

namespace test::tools
{

namespace
{

struct Node final
{
    constexpr Node()
        : info{
            .type = FileType::Directory,
            .size = 4 * KiB,
            .modificationTime = time(nullptr),
        }
        , mappedCount(0)
    {
        utils::constructAt(&directory);
    }

    constexpr Node(void* ptr, size_t size)
        : info{
            .type = FileType::File,
            .size = size,
            .modificationTime = time(nullptr)
        }
        , mappedCount(0)
    {
        utils::constructAt(&file);
        file.data = ptr;
    }

    constexpr Node(std::vector<uint8_t> bytes)
        : info{
            .type = FileType::File,
            .size = bytes.size(),
            .modificationTime = time(nullptr)
        }
        , mappedCount(0)
    {
        utils::constructAt(&file);
        file.ownContent = std::move(bytes);
        file.data = file.ownContent.data();
    }

    constexpr Node(Node&& other)
        : info(other.info)
        , mappedCount(other.mappedCount)
    {
        switch (other.info.type)
        {
            case FileType::File:
                utils::constructAt(&file, std::move(other.file));
                break;
            case FileType::Directory:
                utils::constructAt(&directory, std::move(other.directory));
                break;
            default:
                break;
        }
        other.mappedCount = 0;
    }

    constexpr Node& operator=(Node&& other)
    {
        destroy();
        info = other.info;
        switch (other.info.type)
        {
            case FileType::File:
                utils::constructAt(&file, std::move(other.file));
                break;
            case FileType::Directory:
                utils::constructAt(&directory, std::move(other.directory));
                break;
            default:
                break;
        }
        mappedCount = other.mappedCount;
        other.mappedCount = 0;
        return *this;
    }

    constexpr ~Node()
    {
        destroy();
    }

    FileInfo info;
    union
    {
        struct
        {
            std::vector<uint8_t> ownContent;
            void*                data;
        } file;
        struct
        {
            std::map<std::string, Node> nodes;
        } directory;
    };
    unsigned mappedCount;

private:
    constexpr void destroy()
    {
        switch (info.type)
        {
            case FileType::File:
                utils::destroyAt(&file);
                break;
            case FileType::Directory:
                utils::destroyAt(&directory);
                break;
            default:
                break;
        }
    }
};

}  // namespace

static Node root;
static std::map<void*, Node*> mappedFiles;
static Node* cwd;
static const auto eENOENT = error("No such file or directory");
static const auto eEISDIR = error("Is a directory");
static const auto eENOTDIR = error("Not a directory");

static Node* findInDir(Node* dir, const std::string& name)
{
    ASSERT_THROW(dir->info.type == FileType::Directory, "Not a directory");
    auto it = dir->directory.nodes.find(name);
    if (it == dir->directory.nodes.end()) [[unlikely]]
    {
        return nullptr;
    }
    return &it->second;
}

namespace
{

struct LookupResult final
{
    std::string nodeName;
    Node* node;
    Node* dirNode;
};

}  // namespace

static LookupResult lookup(const std::string_view& path, bool createDirs = false)
{
    Node* node;
    Node* dirNode = nullptr;

    if (path.empty()) [[unlikely]]
    {
        return {};
    }

    if (path[0] == '/')
    {
        node = &root;
    }
    else
    {
        ASSERT_THROW(cwd, "Current working directory not set!");
        node = cwd;
    }

    auto parts = path | utils::splitBy("/");

    for (size_t i = 0; i < parts.size(); ++i)
    {
        dirNode = node;

        auto& part = parts[i];

        if (dirNode->info.type != FileType::Directory) [[unlikely]]
        {
            goto notFound;
        }

        node = findInDir(dirNode, part);

        if (not node) [[unlikely]]
        {
            if (i != parts.size() - 1)
            {
                if (createDirs)
                {
                    auto res = dirNode->directory.nodes.emplace(std::move(part), Node());
                    node = &res.first->second;
                }
                else
                {
                    goto notFound;
                }
            }
        }
    }

    return LookupResult{
        .nodeName = parts.empty() ? "" : std::move(parts.back()),
        .node = node,
        .dirNode = dirNode,
    };

notFound:
    return LookupResult{
        .nodeName{},
        .node = nullptr,
        .dirNode = nullptr,
    };
}

static MaybeFileInfo readFileInfo(const char* pathname)
{
    auto result = lookup(pathname);

    if (not result.node) [[unlikely]]
    {
        return eENOENT;
    }

    return result.node->info;
}

static MaybeError writeToFile(const char* pathname, const void* data, size_t size)
{
    auto result = lookup(pathname);

    if (not result.dirNode) [[unlikely]]
    {
        return eENOENT;
    }

    const auto bytes = static_cast<const uint8_t*>(data);

    std::vector<uint8_t> vec;
    vec.reserve(size);
    vec.insert(vec.end(), bytes, bytes + size);

    result.dirNode->directory.nodes[std::move(result.nodeName)] = Node(std::move(vec));

    return {};
}

static MaybeMapped mapFileImpl(const char* pathname, bool)
{
    auto result = lookup(pathname);

    if (not result.node) [[unlikely]]
    {
        return eENOENT;
    }

    if (result.node->info.type != FileType::File) [[unlikely]]
    {
        return eEISDIR;
    }

    mappedFiles[result.node->file.data] = result.node;
    result.node->mappedCount++;

    return Mapped{
        result.node->file.data,
        result.node->info.size
    };
}

static MaybeError unmapFileImpl(void* ptr, size_t size)
{
    auto it = mappedFiles.find(ptr);
    if (it != mappedFiles.end() and --it->second->mappedCount == 0)
    {
        ASSERT_THROW(it->second->info.size == size, "Invalid size passed; got %zu, expected %zu", size, it->second->info.size);
        mappedFiles.erase(it);
    }
    return {};
}

namespace
{

struct ReadDirContext final
{
    const Node& dirNode;
    decltype(Node::directory.nodes.cbegin()) it;
};

}  // namespace

static MaybeDirEntry readDirectory(const char* pathname, void** ptr)
{
    if (not *ptr)
    {
        auto result = lookup(pathname);

        if (not result.node) [[unlikely]]
        {
            *ptr = nullptr;
            return eENOENT;
        }

        const auto& node = *result.node;

        if (node.info.type != FileType::Directory) [[unlikely]]
        {
            return eENOTDIR;
        }

        if (node.directory.nodes.empty()) [[unlikely]]
        {
            *ptr = nullptr;
            return {};
        }

        auto ctx = new ReadDirContext(node, node.directory.nodes.begin());

        const auto& name = ctx->it->first;
        const auto& info = ctx->it->second.info;
        ++ctx->it;

        *ptr = ctx;

        return DirEntry{
            .name = name.c_str(),
            .info = info,
        };
    }

    auto ctx = static_cast<ReadDirContext*>(*ptr);

    if (ctx->it == ctx->dirNode.directory.nodes.end())
    {
        *ptr = nullptr;
        delete ctx;
        return {};
    }

    const auto& name = ctx->it->first;
    const auto& info = ctx->it->second.info;
    ++ctx->it;

    return DirEntry{
        .name = name.c_str(),
        .info = info,
    };
}

PlatformFake::PlatformFake()
{
    sys::platform.writeToFile   = &writeToFile;
    sys::platform.readFileInfo  = &readFileInfo;
    sys::platform.mapFileImpl   = &mapFileImpl;
    sys::platform.unmapFileImpl = &unmapFileImpl;
    sys::platform.readDirectory = &readDirectory;
}

PlatformFake::~PlatformFake()
{
    root = Node();
    mappedFiles.clear();
    cwd = nullptr;
}

PlatformFake& PlatformFake::addFile(const std::string_view& path)
{
    addFile(path, nullptr, 0);
    return *this;
}

PlatformFake& PlatformFake::addFile(const std::string_view& path, void* data, size_t size)
{
    auto result = lookup(path, true);
    ASSERT_THROW(result.dirNode, "Failed to lookup/create directories");
    auto res = result.dirNode->directory.nodes.emplace(std::move(result.nodeName), Node(data, size));
    ASSERT_THROW(res.second, "%s already exists!", res.first->first.c_str());
    return *this;
}

PlatformFake& PlatformFake::addFile(const std::string_view& path, const std::string_view& sv)
{
    return addFile(path, (void*)sv.data(), sv.size());
}

PlatformFake& PlatformFake::addDirectory(const std::string_view& path)
{
    auto result = lookup(path, true);
    ASSERT_THROW(result.dirNode, "Failed to lookup/create directories");
    result.dirNode->directory.nodes.emplace(std::move(result.nodeName), Node());
    return *this;
}

PlatformFake& PlatformFake::setWorkingDirectory(const std::string_view& path)
{
    auto result = lookup(path);
    ASSERT_THROW(result.node, "Path does not exist!");
    cwd = result.node;
    return *this;
}

}  // namespace test::tools
