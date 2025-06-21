#pragma once

#include <thread>
#include <vector>

namespace atlas::core {
class ThreadPool final {
  public:
    ThreadPool();
    ~ThreadPool() = default;

    ThreadPool(const ThreadPool&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;

    ThreadPool(ThreadPool&&) = delete;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;

  private:
    std::vector<std::thread> workers;
};
} // namespace atlas::core
