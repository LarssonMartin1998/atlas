#pragma once

#include "IStartShutdown.hpp"

namespace atlas::core {
class IGame : public IStartShutdown {
  public:
    ~IGame() override = default;

    IGame(const IGame &) = delete;
    auto operator=(const IGame &) -> IGame & = delete;

    IGame(IGame &&) = delete;
    auto operator=(IGame &&) -> IGame & = delete;

    [[nodiscard]] virtual auto should_quit() const -> bool = 0;

  protected:
    IGame() = default;
};
} // namespace atlas::core
