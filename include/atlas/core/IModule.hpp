#pragma once

#include "IEngine.hpp"

namespace atlas::core {
class IModule {
  public:
    virtual ~IModule() = default;

    IModule(const IModule &) = delete;
    auto operator=(const IModule &) -> IModule & = delete;

    IModule(IModule &&) = delete;
    auto operator=(IModule &&) -> IModule & = delete;

    [[nodiscard]] virtual auto get_engine() const -> IEngine & = 0;

    virtual auto start() -> void = 0;
    virtual auto shutdown() -> void = 0;

  protected:
    IModule() = default;
};
} // namespace atlas::core
