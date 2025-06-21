#pragma once

#include "core/threads/IThreadPool.hpp"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace atlas::core {
class ThreadPool final : public IThreadPool {
  public:
    ThreadPool();
    ~ThreadPool() override = default;

    ThreadPool(const ThreadPool&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;

    ThreadPool(ThreadPool&&) = delete;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;

    auto stop() -> void override;
    auto enqueue(std::function<void()> task) -> void override;
    auto await_all_tasks_completed() -> void override;

  private:
    auto task_rdy_or_stop_signal_condition() -> bool;
    auto all_tasks_completed_condition() -> bool;

    bool should_stop = false;
    std::uint32_t active_tasks = 0;

    std::vector<std::thread> workers;

    std::mutex queue_mutex;
    std::condition_variable await_task_rdy_or_stop_signal;
    std::condition_variable await_all_tasks_finished;
    std::queue<std::function<void()>> tasks;
};
} // namespace atlas::core
