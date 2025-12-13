#pragma once

#include <mutex>
#include <condition_variable>

namespace Vy
{
    // --------------------------------------------------------------------------------------------

    using Mutex = std::mutex;

    // --------------------------------------------------------------------------------------------

    template <typename TMutex = Mutex >
    using LockGuard = std::lock_guard<TMutex>;

    template <typename TMutex = Mutex >
    using UniqueLock = std::unique_lock<TMutex>;

    // --------------------------------------------------------------------------------------------

    using ConditionVariable = std::condition_variable;

    // --------------------------------------------------------------------------------------------

}