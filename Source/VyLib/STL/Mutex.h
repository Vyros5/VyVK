#pragma once

#include <mutex>
#include <condition_variable>
#include <future>

namespace Vy
{
    // --------------------------------------------------------------------------------------------

    using TMutex = std::mutex;

    // --------------------------------------------------------------------------------------------

    template <typename mutex_t = TMutex >
    using TLockGuard = std::lock_guard<mutex_t>;

    template <typename mutex_t = TMutex >
    using TUniqueLock = std::unique_lock<mutex_t>;

    template <typename mutex_t = TMutex >
    using TScopedLock = std::scoped_lock<mutex_t>;

    // --------------------------------------------------------------------------------------------

    using TCondVariable = std::condition_variable;

    // --------------------------------------------------------------------------------------------

    template <typename T >
    using TFuture = std::future<T>;
}