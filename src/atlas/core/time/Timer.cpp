#include "core/time/Timer.hpp"

namespace atlas::core {
auto Timer::elapsed() const -> double {
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(now - start).count();
}

auto Timer::reset() -> void {
    start = std::chrono::high_resolution_clock::now();
}
} // namespace atlas::core
