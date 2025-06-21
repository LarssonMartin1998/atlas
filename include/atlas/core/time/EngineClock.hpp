#pragma once

#include "IEngineClock.hpp"
#include <core/time/Timer.hpp>

namespace atlas::core {
class EngineClock final : public IEngineClock {
  public:
    auto update_delta_time() -> void;

    [[nodiscard]] auto get_total_time() const -> double override;
    [[nodiscard]] auto get_delta_time() const -> double override;

  private:
    struct DeltaTime {
        Timer frame_timer{};
        double previous_time{0.0};
    };

    Timer total_run_time{};
    DeltaTime delta_time{};
};
} // namespace atlas::core
