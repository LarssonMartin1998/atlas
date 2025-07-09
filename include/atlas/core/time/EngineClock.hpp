#pragma once

#include <limits>
#include <optional>

#include "IEngineClock.hpp"

#include "core/time/Timer.hpp"

namespace atlas::core {
class EngineClock final : public IEngineClock {
  public:
    auto update_frame_timers(const std::uint64_t& num_frames) -> void;

    [[nodiscard]] auto get_total_time_without_first_frame() const
        -> std::expected<double, EngineClockErrorCode> override;
    [[nodiscard]] auto get_total_time() const -> double override;
    [[nodiscard]] auto get_delta_time() const -> double override;

    [[nodiscard]] auto get_avg_frame_time() const -> double override;
    [[nodiscard]] auto get_fastest_frame_time() const -> double override;
    [[nodiscard]] auto get_slowest_frame_time() const -> double override;

    auto start_post_first_frame_timer() -> void;

  private:
    struct DeltaTime {
        Timer frame_timer{};
        double previous_time{0.0};
    };

    Timer total_run_time{};
    std::optional<Timer> run_time_post_first_frame;
    DeltaTime delta_time{};

    double avg_frame_time{0.0};
    double fastest_frame{std::numeric_limits<double>::max()};
    double slowest_frame{0.0};
};
} // namespace atlas::core
