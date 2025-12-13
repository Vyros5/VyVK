#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Core/Assert.h>
#include <VyLib/STL/Mutex.h>
#include <VyLib/STL/Containers.h>
#include <VyLib/STL/Utility.h>
#include <VyLib/STL/Atomic.h>

#include <future>
#include <condition_variable>
#include <thread>

namespace Vy
{
    class ExecutorService 
    {
    public:
        /**
         * @brief Constructs an executor service with the specified number of threads
         * @param numThreads Number of worker threads (0 = auto-detect based on hardware)
         */
        explicit ExecutorService(size_t numThreads = 0);

        /**
         * @brief Destructor - stops all threads and waits for them to finish
         */
        ~ExecutorService();

        /**
         * @brief Submit a task to the executor service
         * @param f Function to execute
         * @param args Arguments to pass to the function
         * @return Future that will contain the result of the function
         */
        template<typename F, typename... Args>
        auto submit(F &&f, Args &&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

        /**
         * @brief Get the number of worker threads in the pool
         * @return Number of threads
         */
        size_t getThreadCount() const;

        /**
         * @brief Get the number of pending tasks in the queue
         * @return Number of queued tasks
         */
        size_t getQueuedTaskCount() const;

        // Delete copy operations
        ExecutorService(const ExecutorService&) = delete;
        ExecutorService& operator=(const ExecutorService&) = delete;

        // Allow move operations
        ExecutorService(ExecutorService&&) noexcept = default;
        ExecutorService& operator=(ExecutorService&&) noexcept = default;

    private:
        TVector<std::thread>          m_Workers;
        TQueue<std::function<void()>> m_Tasks;

        mutable std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_bStop = false;
    };

    template<typename F, typename... Args>
    auto ExecutorService::submit(F &&f, Args &&... args) -> std::future<typename std::invoke_result<F, Args...>::type> 
    {
        using ReturnType = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future(); 
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            if (m_bStop) 
            {
                throw std::runtime_error("Cannot submit task to stopped ExecutorService");
            }

            m_Tasks.emplace([task]() { (*task)(); });
        }

        m_Condition.notify_one();

        return result;
    }
}