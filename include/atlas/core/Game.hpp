#pragma once

#include "core/IEngine.hpp"
#include "core/IGame.hpp"

namespace atlas::core {
class Game : virtual public IGame {
  public:
    [[nodiscard]] auto get_engine() const -> IEngine& override;

    auto set_engine(IEngine& engine_ref) -> void override;

  private:
    IEngine* engine;
};
} // namespace atlas::core
