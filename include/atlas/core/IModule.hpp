#pragma once

#include "IEngine.hpp"
#include "IStartShutdown.hpp"

namespace atlas::core {
class IModule : public IStartShutdown {
  public:
    ~IModule() override = default;

    IModule(const IModule &) = delete;
    auto operator=(const IModule &) -> IModule & = delete;

    IModule(IModule &&) = delete;
    auto operator=(IModule &&) -> IModule & = delete;

    [[nodiscard]] virtual auto get_engine() const -> IEngine & = 0;

  protected:
    IModule() = default;
};
} // namespace atlas::core
