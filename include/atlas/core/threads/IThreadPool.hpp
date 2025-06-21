#pragma once

#include <functional>
namespace atlas::core {
class IThreadPool {
  public:
    virtual ~IThreadPool() = default;

    IThreadPool(const IThreadPool&) = delete;
    auto operator=(const IThreadPool&) -> IThreadPool& = delete;

    IThreadPool(IThreadPool&&) = delete;
    auto operator=(IThreadPool&&) -> IThreadPool& = delete;

    virtual auto stop() -> void = 0;
    virtual auto enqueue(std::function<void()> task) -> void = 0;
    virtual auto await_all_tasks_completed() -> void = 0;

  protected:
    IThreadPool() = default;
};
} // namespace atlas::core
