#pragma once

#include <ctime>
#include <string>

#include <fmt/format.h>

#include "severity.hpp"
#include "utils/immobile.hpp"
#include "utils/inline.hpp"
#include "utils/source_location.hpp"

namespace core
{

struct LogEntry final
{
    Severity              severity;
    std::time_t           time;
    utils::SourceLocation location;
    const char*           header;
    std::string           message;
};

#ifndef LOG_HEADER
#define LOG_HEADER nullptr
#endif

struct Logger final
{
    struct Flusher final : utils::Immobile
    {
        Flusher(
            Severity severity,
            const char* header,
            utils::SourceLocation loc,
            std::string& buffer);

        ALWAYS_INLINE constexpr ~Flusher()
        {
            Logger::registerLogEntry(mSeverity, mHeader, mLocation);
        }

        template <typename ...Args>
        ALWAYS_INLINE void write(fmt::format_string<Args...> fmt, Args&&... args)
        {
            mBuffer = fmt::format(std::move(fmt), std::forward<Args>(args)...);
        }

        ALWAYS_INLINE auto& buffer()
        {
            return mBuffer;
        }

    private:
        const Severity              mSeverity;
        const char* const           mHeader;
        const utils::SourceLocation mLocation;
        std::string&                mBuffer;
    };

    constexpr static Flusher debug(
        utils::SourceLocation loc = utils::SourceLocation::current(),
        const char* header = LOG_HEADER)
    {
        return log(Severity::debug, header, loc);
    }

    constexpr static Flusher info(
        utils::SourceLocation loc = utils::SourceLocation::current(),
        const char* header = LOG_HEADER)
    {
        return log(Severity::info, header, loc);
    }

    constexpr static Flusher notice(
        utils::SourceLocation loc = utils::SourceLocation::current(),
        const char* header = LOG_HEADER)
    {
        return log(Severity::notice, header, loc);
    }

    constexpr static Flusher warning(
        utils::SourceLocation loc = utils::SourceLocation::current(),
        const char* header = LOG_HEADER)
    {
        return log(Severity::warning, header, loc);
    }

    constexpr static Flusher error(
        utils::SourceLocation loc = utils::SourceLocation::current(),
        const char* header = LOG_HEADER)
    {
        return log(Severity::error, header, loc);
    }

    static Flusher log(
        Severity severity,
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current());

private:
    static void registerLogEntry(
        Severity severity,
        const char* header,
        utils::SourceLocation loc);
};

constexpr static Logger logger;

}  // namespace core
