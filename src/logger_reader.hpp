#pragma once

#include <functional>

#include "logger.hpp"
#include "utils/function_ref.hpp"

struct LoggerReader final
{
    using OnLogCallback = std::move_only_function<void(const LogEntry&)>;
    static void forEachLogEntry(utils::FunctionRef<void(const LogEntry&)> visitor);
    static void onLog(OnLogCallback callback);
};

constexpr static LoggerReader loggerReader;
