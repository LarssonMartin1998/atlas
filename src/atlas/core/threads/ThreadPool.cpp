#include "core/threads/ThreadPool.hpp"

#include <cassert>
#include <thread>

namespace atlas::core {
ThreadPool::ThreadPool() {
    workers.reserve([]() {
        const auto num = std::thread::hardware_concurrency();
        return num == 0 ? 1 : num;
    }());

    for (size_t i = 0; i < workers.capacity(); ++i) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);

                    await_task_rdy_or_stop_signal.wait(lock, [this]() {
                        return task_rdy_or_stop_signal_condition();
                    });

                    if (should_stop && tasks.empty()) {
                        break;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                    ++active_tasks;
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    --active_tasks;

                    if (all_tasks_completed_condition()) {
                        await_all_tasks_finished.notify_all();
                    }
                }
            }
        });
    }
}

auto ThreadPool::stop() -> void {
    assert(!should_stop && "ThreadPool is already stopped");

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_stop = true;
    }

    await_task_rdy_or_stop_signal.notify_all();
    for (auto& worker : workers) {
        worker.join();
    }
}

auto ThreadPool::enqueue(std::function<void()> task) -> void {
    assert(!should_stop && "ThreadPool is stopped");

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }

    await_task_rdy_or_stop_signal.notify_one();
}

auto ThreadPool::await_all_tasks_completed() -> void {
    std::unique_lock<std::mutex> lock(queue_mutex);
    await_all_tasks_finished.wait(
        lock, [this]() { return all_tasks_completed_condition(); });
}

auto ThreadPool::task_rdy_or_stop_signal_condition() -> bool {
    return should_stop || !tasks.empty();
}

auto ThreadPool::all_tasks_completed_condition() -> bool {
    return tasks.empty() && active_tasks == 0;
}
} // namespace atlas::core
