#include "core/time/EngineClock.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>

namespace atlas::core {
auto EngineClock::update_frame_timers(const std::uint64_t& num_frames) -> void {
    delta_time.previous_time = delta_time.frame_timer.elapsed();
    delta_time.frame_timer.reset();

    if (run_time_post_first_frame.has_value()) {
        avg_frame_time = (*run_time_post_first_frame).elapsed() / static_cast<double>(num_frames);
        fastest_frame = std::min(delta_time.previous_time, fastest_frame);
        slowest_frame = std::max(delta_time.previous_time, slowest_frame);
    }
}

auto EngineClock::get_total_time_without_first_frame() const
    -> std::expected<double, EngineClockErrorCode> {
    if (run_time_post_first_frame.has_value()) {
        return (*run_time_post_first_frame).elapsed();
    }

    return std::unexpected(EngineClockErrorCode::FirstFrameHasNotFinishedOrTimerNotStarted);
}

auto EngineClock::get_total_time() const -> double {
    return total_run_time.elapsed();
}

auto EngineClock::get_delta_time() const -> double {
    return delta_time.previous_time;
}

auto EngineClock::get_avg_frame_time() const -> double {
    return avg_frame_time;
}

auto EngineClock::get_fastest_frame_time() const -> double {
    return fastest_frame;
}

auto EngineClock::get_slowest_frame_time() const -> double {
    return slowest_frame;
}

auto EngineClock::start_post_first_frame_timer() -> void {
    assert(
        !run_time_post_first_frame.has_value()
        && "Trying to start post first frame timer which is already started"
    );

    run_time_post_first_frame = Timer{};
}
} // namespace atlas::core
