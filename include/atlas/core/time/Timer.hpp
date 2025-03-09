#pragma once

#include <chrono>

namespace atlas::core {
class Timer final {
  public:
    [[nodiscard]] auto elapsed() const -> double;
    auto reset() -> void;

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start{
        std::chrono::high_resolution_clock::now()};
};
} // namespace atlas::core
