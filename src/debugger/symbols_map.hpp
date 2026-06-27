#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <atomic>

namespace debugger
{

struct Context;

struct Symbol
{
    uint16_t    bank;
    uint16_t    start;
    uint16_t    size;
    std::string name;
};

struct SymbolsMap
{
    SymbolsMap();
    ~SymbolsMap();
    void initialize(Context& ctx);
    const Symbol* operator[](uint16_t address) const;
    const Symbol* operator[](uint16_t bank, uint16_t address) const;
    const Symbol* operator[](const std::string_view& name) const;

private:
    void loadSymbols(Context& ctx);
    struct Impl;
    std::atomic<Impl*> mPimpl;
};

}  // namespace debugger
