#pragma once

#include "core/IEngine.hpp"
#include "core/IGame.hpp"

namespace atlas::core {
class Game : virtual public IGame {
  public:
    [[nodiscard]] auto get_engine() const -> IEngine& override;

  protected:
    auto set_engine(std::weak_ptr<IEngine> engine_ptr) -> void override;

  private:
    std::weak_ptr<IEngine> engine;
};
} // namespace atlas::core
