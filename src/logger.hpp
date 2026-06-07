#pragma once

#include <ctime>
#include <string>

#include <fmt/fmt_ext.h>

#include "utils/bitflag.hpp"
#include "utils/immobile.hpp"
#include "utils/inline.hpp"
#include "utils/source_location.hpp"

enum Severity : char
{
    debug,
    info,
    warning,
    error,
};

DEFINE_BITFLAG(LogEntryFlags, char,
{
    noSourceLocation,
});

struct LogEntry final
{
    using Flags = LogEntryFlags;

    Severity              severity;
    Flags                 flags;
    std::time_t           time;
    utils::SourceLocation location;
    const char*           header;
    std::string           message;
};

#ifndef LOG_HEADER
#define LOG_HEADER nullptr
#endif

#ifndef LOG_FLAGS
#define LOG_FLAGS
#endif

struct Logger final
{
    struct Flusher final : utils::Immobile
    {
        Flusher(
            Severity severity,
            LogEntryFlags flags,
            const char* header,
            utils::SourceLocation loc,
            std::string& buffer);

        ALWAYS_INLINE constexpr ~Flusher()
        {
            Logger::registerLogEntry(mSeverity, mFlags, mHeader, mLocation);
        }

        template <typename ...Args>
        ALWAYS_INLINE void write(fmt::format_string<Args...> fmt, Args&&... args)
        {
            mBuffer = fmt::format_to_string(std::move(fmt), std::forward<Args>(args)...);
        }

        ALWAYS_INLINE auto& buffer()
        {
            return mBuffer;
        }

    private:
        const Severity              mSeverity;
        const LogEntryFlags         mFlags;
        const char* const           mHeader;
        const utils::SourceLocation mLocation;
        std::string&                mBuffer;
    };

    constexpr static Flusher debug(
        LogEntryFlags flags = LogEntryFlags(LOG_FLAGS),
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current())
    {
        return log(Severity::debug, flags, header, loc);
    }

    constexpr static Flusher info(
        LogEntryFlags flags = LogEntryFlags(LOG_FLAGS),
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current())
    {
        return log(Severity::info, flags, header, loc);
    }

    constexpr static Flusher warning(
        LogEntryFlags flags = LogEntryFlags(LOG_FLAGS),
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current())
    {
        return log(Severity::warning, flags, header, loc);
    }

    constexpr static Flusher error(
        LogEntryFlags flags = LogEntryFlags(LOG_FLAGS),
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current())
    {
        return log(Severity::error, flags, header, loc);
    }

    static Flusher log(
        Severity severity,
        LogEntryFlags flags = LogEntryFlags(LOG_FLAGS),
        const char* header = LOG_HEADER,
        utils::SourceLocation loc = utils::SourceLocation::current());

private:
    static void registerLogEntry(
        Severity severity,
        LogEntryFlags flags,
        const char* header,
        utils::SourceLocation loc);
};

constexpr static Logger logger;
