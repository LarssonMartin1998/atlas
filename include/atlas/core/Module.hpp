#pragma once

#include "IModule.hpp"

namespace atlas::core {
class IEngine;
}

namespace atlas::core {
class Module : public virtual IModule {
  public:
    explicit Module(IEngine& engine);

  protected:
    [[nodiscard]] auto get_engine() const -> IEngine& override;

    auto start() -> void override;
    auto shutdown() -> void override;

  private:
    IEngine& engine;
};
} // namespace atlas::core
