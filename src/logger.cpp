#include "logger.hpp"

#include <ctime>
#include <mutex>

#include "logger_reader.hpp"
#include "utils/ring_buffer.hpp"
#include "utils/source_location.hpp"

using LogRingBuffer = utils::RingBuffer<LogEntry>;

namespace
{

struct LoggerState final
{
    std::mutex       lock;
    LogRingBuffer    ringBuffer{1024};
};

}  // namespace

static LoggerState state;
static thread_local std::string buffer;
static LoggerReader::OnLogCallback onLogCallback;

Logger::Flusher Logger::log(Severity severity, const char* header, utils::SourceLocation loc)
{
    return Flusher(severity, header, loc, buffer);
}

void Logger::registerLogEntry(Severity severity, const char* header, utils::SourceLocation loc)
{
    LogEntry entry{
        .severity = severity,
        .time = std::time(nullptr),
        .location = loc,
        .header = header,
        .message = std::move(buffer),
    };

    std::scoped_lock scopedLock(state.lock);

    if (onLogCallback)
    {
        onLogCallback(entry);
    }

    state.ringBuffer.pushBack(std::move(entry));
}

Logger::Flusher::Flusher(
    Severity severity,
    const char* header,
    utils::SourceLocation loc,
    std::string& buffer)
    : mSeverity(severity)
    , mHeader(header)
    , mLocation(loc)
    , mBuffer(buffer)
{
}

void LoggerReader::forEachLogEntry(utils::FunctionRef<void(const LogEntry&)> visitor)
{
    state.ringBuffer.forEach(visitor);
}

void LoggerReader::onLog(OnLogCallback callback)
{
    onLogCallback = std::move(callback);
}
