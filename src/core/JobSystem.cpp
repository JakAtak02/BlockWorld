#include "core/JobSystem.h"

#include <iostream>

JobSystem::JobSystem()
{
}

JobSystem::~JobSystem()
{
    shutdown();
}

void JobSystem::initialize(
    size_t workerThreadCount
)
{
    if (m_running)
    {
        return;
    }

    m_running = true;

    for (size_t i = 0;
        i < workerThreadCount;
        i++)
    {
        m_workerThreads.emplace_back(
            &JobSystem::workerLoop,
            this
        );
    }

    std::cout
        << "JobSystem initialized with "
        << workerThreadCount
        << " worker threads."
        << std::endl;
}

void JobSystem::shutdown()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;

    m_conditionVariable.notify_all();

    for (std::thread& thread :
        m_workerThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    m_workerThreads.clear();

    std::cout
        << "JobSystem shutdown complete."
        << std::endl;
}

void JobSystem::submit(Job job)
{
    {
        std::lock_guard<std::mutex>
            lock(m_queueMutex);

        m_jobQueue.push(
            std::move(job)
        );
    }

    m_conditionVariable.notify_one();
}

void JobSystem::workerLoop()
{
    while (m_running)
    {
        Job job;

        {
            std::unique_lock<std::mutex>
                lock(m_queueMutex);

            m_conditionVariable.wait(
                lock,
                [this]()
                {
                    return
                        !m_running ||
                        !m_jobQueue.empty();
                }
            );

            if (!m_running &&
                m_jobQueue.empty())
            {
                return;
            }

            job =
                std::move(
                    m_jobQueue.front()
                );

            m_jobQueue.pop();
        }

        if (job)
        {
            job();
        }
    }
}