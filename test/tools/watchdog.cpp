#include "watchdog.hpp"

#include <atomic>
#include <cstdlib>
#include <mutex>
#include <thread>

#include "src/sys/platform.hpp"

using namespace std::chrono_literals;

namespace test::tools
{

static std::thread watchdogThread;
static std::atomic_bool enabledFlag;
static std::atomic_bool stopFlag;
static std::mutex mutex;
double watchdogTimeout = 0;

static void watchdog()
{
    double sum = 0;
    while (1)
    {
        if (stopFlag)
        {
            break;
        }

        std::this_thread::sleep_for(100ms);

        bool abort = false;
        if (enabledFlag)
        {
            sum += 0.1;
            std::scoped_lock lock(mutex);
            if (sum >= watchdogTimeout) [[unlikely]]
            {
                abort = true;
            }
        }
        else
        {
            sum = 0;
        }

        if (abort) [[unlikely]]
        {
            sys::abortMainThread();
        }
    }
}

void createWatchdog()
{
    watchdogThread = std::thread(&watchdog);
    atexit(&deleteWatchdog);
}

void deleteWatchdog()
{
    stopFlag = true;
    if (watchdogThread.joinable())
    {
        watchdogThread.join();
    }
}

void withWatchdog(double timeout, utils::FunctionRef<void()> task)
{
    {
        std::scoped_lock lock(mutex);
        watchdogTimeout = timeout;
        enabledFlag = true;
    }

    task();

    {
        std::scoped_lock lock(mutex);
        enabledFlag = false;
        watchdogTimeout = 0;
    }
}

}  // namespace test::tools
