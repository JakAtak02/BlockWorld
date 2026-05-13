#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class JobSystem
{
public:
    using Job = std::function<void()>;

public:
    JobSystem();

    ~JobSystem();

    void initialize(
        size_t workerThreadCount
    );

    void shutdown();

    void submit(Job job);

private:
    void workerLoop();

private:
    std::vector<std::thread>
        m_workerThreads;

    std::queue<Job>
        m_jobQueue;

    std::mutex m_queueMutex;

    std::condition_variable
        m_conditionVariable;

    std::atomic<bool>
        m_running = false;
};