#pragma once

#include <mutex>
#include <optional>
#include <queue>

template<typename T>
class ThreadSafeQueue
{
public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.push(
            std::move(value)
        );
    }

    std::optional<T> tryPop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_queue.empty())
        {
            return std::nullopt;
        }

        T value =
            std::move(
                m_queue.front()
            );

        m_queue.pop();

        return value;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_queue.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        return m_queue.size();
    }

private:
    mutable std::mutex m_mutex;

    std::queue<T> m_queue;
};