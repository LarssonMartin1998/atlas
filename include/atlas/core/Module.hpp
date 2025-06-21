#pragma once

#include "core/IModule.hpp"

namespace atlas::core {
class IEngine;
}

namespace atlas::core {
class Module : public virtual IModule {
  public:
    explicit Module(IEngine& engine);

    [[nodiscard]] auto get_engine() const -> IEngine& override;

  protected:
    auto pre_start() -> void override;
    auto start() -> void override;
    auto post_start() -> void override;

    auto pre_shutdown() -> void override;
    auto shutdown() -> void override;
    auto post_shutdown() -> void override;

  private:
    IEngine& engine;
};
} // namespace atlas::core
