#pragma once

#include <mutex>
#include <condition_variable>

template <typename T>
class AsyncResult {

private:
    std::mutex resultMutex;
    std::condition_variable resultConditionVariable;
    bool hasResult = false;
    T result;

public:
    T Get() {
        std::unique_lock<std::mutex> lock(resultMutex);
        resultConditionVariable.wait(lock, [this] { return hasResult; });
        return result;
    }
    void Set(T const& result) {
        std::unique_lock<std::mutex> lock(resultMutex);
        this->result = result;
        hasResult = true;
        lock.unlock();
        resultConditionVariable.notify_all();
        lock.lock();
    }
    void Clear() {
        resultMutex.lock();
        hasResult = false;
        resultMutex.unlock();
    }

};