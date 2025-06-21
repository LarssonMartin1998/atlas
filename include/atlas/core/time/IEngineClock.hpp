#pragma once

namespace atlas::core {
class IEngineClock {
  public:
    virtual ~IEngineClock() = default;

    IEngineClock(const IEngineClock&) = delete;
    auto operator=(const IEngineClock&) -> IEngineClock& = delete;

    IEngineClock(IEngineClock&&) = delete;
    auto operator=(IEngineClock&&) -> IEngineClock& = delete;

    [[nodiscard]] virtual auto get_total_time() const -> double = 0;
    [[nodiscard]] virtual auto get_delta_time() const -> double = 0;

  protected:
    IEngineClock() = default;
};
} // namespace atlas::core
