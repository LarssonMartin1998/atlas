#include "core/time/EngineClock.hpp"

namespace atlas::core {
auto EngineClock::update_delta_time() -> void {
    delta_time.previous_time = delta_time.frame_timer.elapsed();
    delta_time.frame_timer.reset();
}

auto EngineClock::get_total_time() const -> double {
    return total_run_time.elapsed();
}

auto EngineClock::get_delta_time() const -> double {
    return delta_time.previous_time;
}
} // namespace atlas::core
