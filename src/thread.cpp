#include "thread.hpp"

#include <thread>

using Thread = std::thread;

static const auto mainThreadId = std::this_thread::get_id();

void async(Task task)
{
    std::thread(std::move(task)).detach();
}
