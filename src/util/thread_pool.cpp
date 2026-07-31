#include "thread_pool.hpp"
#include "engine/logger.hpp"
#include <mutex>
#include <thread>

Task::Task(int priority, std::function<void()> function, u64 added)
    : priority{priority},
    added{added},
    function{function}
{}

std::strong_ordering Task::operator<=>(const Task& other) const {
    if (priority != other.priority) {
        return priority <=> other.priority;
    }
    return other.added <=> added;
}

void ThreadPool::workerThread() {
    for (;;) {
        std::unique_lock lock{tasksMutex};
        alertThreads.wait(lock, [this](){
            return !tasks.empty() || terminating;
        });

        if (tasks.empty() && terminating) {
            return;
        }

        Task task = tasks.top();
        tasks.pop();
        lock.unlock();

        task.function();
    }
}

ThreadPool::ThreadPool(int threadCount) {
    for (int i = 0; i < threadCount; ++i) {
        std::thread thread{[this](){ this->workerThread(); }};
        Logger::debug(std::format("Started thread {:x}", thread.native_handle()));
        threads.push_back(std::move(thread));
    }
}

ThreadPool::~ThreadPool() {
    terminate();
}

void ThreadPool::enqueueTask(int priority, std::function<void()> function) {
    // Keep the same interface when running on a single thread
    if (threads.empty()) {
        function();
        return;
    }

    {
        std::lock_guard lock{tasksMutex};
        tasks.push(Task{priority, function, totalTasksAdded++});
        alertThreads.notify_one();
    }
}

void ThreadPool::terminate() {
    if (terminating) {
        return;
    }

    {
        std::lock_guard lock{tasksMutex};
        terminating = true;
        alertThreads.notify_all();
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
    threads.clear();
}
