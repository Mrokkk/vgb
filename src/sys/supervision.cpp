#include "supervision.hpp"

#include <atomic>
#include <thread>

#include <fmt/base.h>

#include "sys/platform.hpp"

namespace sys
{

static std::atomic_int counter = 0;
static std::thread supervisionThread;
static std::atomic_bool stop = false;

static void supervision()
{
    using namespace std::chrono_literals;

    int failed = 0;
    while (1)
    {
        std::this_thread::sleep_for(200ms);

        if (stop)
        {
            break;
        }

        if (counter == 0)
        {
            if (++failed == 5)
            {
                fmt::println("Main thread is not responding");
                platform.abort();
                break;
            }
        }
        else
        {
            counter = 0;
            failed = 0;
        }
    }
}

static void finishSupervision()
{
    stop = true;
    if (supervisionThread.joinable())
    {
        supervisionThread.join();
    }
}

void initSupervision()
{
    supervisionThread = std::thread(&supervision);
    atexit(&finishSupervision);
}

void pingSupervision()
{
    counter++;
}

void stopSupervision()
{
    stop = true;
}

}  // namespace sys
