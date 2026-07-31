#ifndef FAKECRAFT_THREAD_POOL_HPP
#define FAKECRAFT_THREAD_POOL_HPP

#include "util/types.hpp"
#include <compare>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

// Priority thread pool
// Based on the desgin from https://stackoverflow.com/questions/15752659/thread-pooling-in-c11 and extended

struct Task {
    int priority = 0;
    u64 added = 0;

    std::function<void()> function;

    Task(int priority, std::function<void()> function, u64 added = 0);

    std::strong_ordering operator<=>(const Task& other) const;
};

class ThreadPool {
private:
    u64 totalTasksAdded = 0;

    std::priority_queue<Task> tasks;
    std::vector<std::thread> threads;

    std::condition_variable alertThreads;
    std::mutex tasksMutex;

    bool terminating = false;

    void workerThread();

public:

    ThreadPool(int threadCount);
    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;

    void enqueueTask(int priority, std::function<void()> function);
    void terminate();
};

#endif
