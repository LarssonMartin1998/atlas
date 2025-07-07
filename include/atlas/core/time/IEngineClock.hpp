#pragma once

#include <cstdint>
#include <expected>

namespace atlas::core {
enum class EngineClockErrorCode : std::uint8_t {
    NoError,

    FirstFrameHasNotFinishedOrTimerNotStarted
};

class IEngineClock {
  public:
    virtual ~IEngineClock() = default;

    IEngineClock(const IEngineClock&) = delete;
    auto operator=(const IEngineClock&) -> IEngineClock& = delete;

    IEngineClock(IEngineClock&&) = delete;
    auto operator=(IEngineClock&&) -> IEngineClock& = delete;

    [[nodiscard]] virtual auto get_total_time_without_first_frame() const
        -> std::expected<double, EngineClockErrorCode> = 0;
    [[nodiscard]] virtual auto get_total_time() const -> double = 0;
    [[nodiscard]] virtual auto get_delta_time() const -> double = 0;

    [[nodiscard]] virtual auto get_avg_frame_time() const -> double = 0;
    [[nodiscard]] virtual auto get_fastest_frame_time() const -> double = 0;
    [[nodiscard]] virtual auto get_slowest_frame_time() const -> double = 0;

  protected:
    IEngineClock() = default;
};
} // namespace atlas::core
