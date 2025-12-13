#include <VyLib/Util/Executor.h>

namespace Vy
{
    ExecutorService::ExecutorService(size_t numThreads) 
    {
        if (numThreads == 0) 
        {
            numThreads = std::thread::hardware_concurrency();
        
            if (numThreads == 0) 
            {
                numThreads = 4;
            }
        }

        m_Workers.reserve(numThreads);

        for (size_t i = 0; i < numThreads; ++i) 
        {
            m_Workers.emplace_back([this] 
            {
                while (true) 
                {
                    std::function<void()> task; 
                    {
                        std::unique_lock<std::mutex> lock(m_QueueMutex);

                        m_Condition.wait(lock, [this] 
                        {
                            return m_bStop || !m_Tasks.empty();
                        });

                        if (m_bStop && m_Tasks.empty()) 
                        {
                            return;
                        }

                        task = std::move(m_Tasks.front());

                        m_Tasks.pop();
                    }

                    task(); // Execute task outside of lock
                }
            });
        }
    }

    ExecutorService::~ExecutorService() 
    { 
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_bStop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker: m_Workers) 
        {
            if (worker.joinable()) 
            {
                worker.join();
            }
        }
    }

    size_t ExecutorService::getThreadCount() const 
    {
        return m_Workers.size();
    }

    size_t ExecutorService::getQueuedTaskCount() const 
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        return m_Tasks.size();
    }
}