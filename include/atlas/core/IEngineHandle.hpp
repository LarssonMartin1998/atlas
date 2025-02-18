#pragma once

namespace atlas::core {
class IEngine;
}

namespace atlas::core {
class IEngineHandle {
  public:
    virtual ~IEngineHandle() = default;

    IEngineHandle(const IEngineHandle&) = delete;
    auto operator=(const IEngineHandle&) -> IEngineHandle& = delete;

    IEngineHandle(IEngineHandle&&) = delete;
    auto operator=(IEngineHandle&&) -> IEngineHandle& = delete;

    [[nodiscard]] virtual auto get_engine() const -> IEngine& = 0;

  protected:
    IEngineHandle() = default;
};
} // namespace atlas::core
