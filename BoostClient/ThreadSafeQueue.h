#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        lock.unlock();
        cond_var_.notify_one();
    }

    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
};
