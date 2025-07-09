#pragma once

namespace atlas::core {
class ITickable {
  public:
    virtual ~ITickable() = default;

    ITickable(const ITickable&) = delete;
    auto operator=(const ITickable&) -> ITickable& = delete;

    ITickable(ITickable&&) = delete;
    auto operator=(ITickable&&) -> ITickable& = delete;

    virtual auto tick() -> void = 0;

  protected:
    ITickable() = default;
};
} // namespace atlas::core
