#pragma once

#include "IStartShutdown.hpp"

namespace atlas::core {
class IEngine;
} // namespace atlas::core

namespace atlas::core {
class IModule : public IStartShutdown {
  public:
    ~IModule() override = default;

    IModule(const IModule&) = delete;
    auto operator=(const IModule&) -> IModule& = delete;

    IModule(IModule&&) = delete;
    auto operator=(IModule&&) -> IModule& = delete;

    [[nodiscard]] virtual auto get_engine() const -> IEngine& = 0;

  protected:
    IModule() = default;
};
} // namespace atlas::core
