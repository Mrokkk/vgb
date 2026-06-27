#include "symbols_map.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>

#include "debugger/context.hpp"
#include "fmt/base.h"
#include "game_boy.hpp"
#include "logger.hpp"
#include "sys/system.hpp"
#include "thread.hpp"
#include "utils/time.hpp"

namespace debugger
{

struct SymbolsMap::Impl
{
    Impl(Context& c)
        : ctx(c)
    {
    }

    Context& ctx;
    std::vector<Symbol> symbols;
    std::map<std::string_view, Symbol*> mapping;
};

SymbolsMap::SymbolsMap() = default;

SymbolsMap::~SymbolsMap()
{
    if (mPimpl)
    {
        delete mPimpl;
    }
}

void SymbolsMap::initialize(Context& ctx)
{
    async([this, &ctx]{ loadSymbols(ctx); });
}

const Symbol* SymbolsMap::operator[](uint16_t address) const
{
    if (not mPimpl) [[unlikely]]
    {
        return nullptr;
    }
    auto& impl = *mPimpl;
    size_t symbolCount = impl.symbols.size();
    for (size_t i = 0; i < symbolCount; ++i)
    {
        const auto& sym = impl.symbols[i];
        if (sym.bank == 0 and sym.start <= address and sym.start + sym.size > address)
        {
            return &sym;
        }
    }
    return nullptr;
}

const Symbol* SymbolsMap::operator[](uint16_t bank, uint16_t address) const
{
    if (not mPimpl) [[unlikely]]
    {
        return nullptr;
    }
    auto& impl = *mPimpl;
    size_t symbolCount = impl.symbols.size();
    for (size_t i = 0; i < symbolCount; ++i)
    {
        const auto& sym = impl.symbols[i];
        if (sym.bank == bank and sym.start <= address and sym.start + sym.size > address)
        {
            return &sym;
        }
    }
    return nullptr;
}

const Symbol* SymbolsMap::operator[](const std::string_view& name) const
{
    if (not mPimpl) [[unlikely]]
    {
        return nullptr;
    }
    auto& impl = *mPimpl;
    auto it = impl.mapping.find(name);
    if (it == impl.mapping.end()) [[unlikely]]
    {
        return nullptr;
    }
    return it->second;
}

void SymbolsMap::loadSymbols(Context& ctx)
{
    const auto t = utils::startTimeMeasurement();

    const auto& romPath = ctx.gb.config.cartridgePath;
    std::filesystem::path p = romPath;
    p.replace_extension(".sym");
    auto file = sys::mapFile(p.c_str());

    if (not file)
    {
        logger.info().write("No debug symbols for {}", romPath);
        return;
    }

    auto impl = new Impl(ctx);
    impl->symbols.reserve(1024);

    std::string_view buf(file->getData<char>(), file->getSize());

    while (not buf.empty())
    {
        auto newline = buf.find('\n');

        if (newline == buf.npos)
        {
            newline = buf.size();
        }

        if (buf[0] == ';')
        {
            buf.remove_prefix(newline + 1);
            continue;
        }

        uint32_t bank;
        uint32_t addr;
        char name[128];

        if (sscanf(buf.data(), "%x:%x %s\n", &bank, &addr, name))
        {
            impl->symbols.push_back(
                Symbol{
                    .bank = static_cast<uint16_t>(bank),
                    .start = static_cast<uint16_t>(addr),
                    .size = 0,
                    .name = name
                });
        }

        buf.remove_prefix(newline + 1);
    }

    Symbol* prevSymbol = nullptr;
    for (auto& symbol : impl->symbols)
    {
        if (prevSymbol)
        {
            if (prevSymbol->start < 0x4000 and prevSymbol->bank == 0)
            {
                prevSymbol->size = std::min(symbol.start, uint16_t(0x4000)) - prevSymbol->start;
            }
            else if (prevSymbol->start >= 0x4000 and prevSymbol->start < 0x8000)
            {
                if (prevSymbol->bank == symbol.bank)
                {
                    prevSymbol->size = symbol.start - prevSymbol->start;
                }
                else
                {
                    prevSymbol->size = 0x8000 - prevSymbol->start;
                }
            }
            else
            {
                prevSymbol->size = symbol.start - prevSymbol->start;
            }
        }

        impl->mapping.emplace(std::string_view(symbol.name), &symbol);

        prevSymbol = &symbol;
    }
    if (prevSymbol)
    {
        prevSymbol->size = 0xffff - prevSymbol->start;
    }

    logger.info().write("Loaded {} symbols for {}; took {:.2f} s", impl->symbols.size(), romPath, t.elapsed());

    mPimpl = impl;
}

}  // namespace debugger
