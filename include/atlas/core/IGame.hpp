#pragma once

namespace atlas::core {
class IGame {
  public:
    virtual ~IGame() = default;

    IGame(const IGame &) = delete;
    auto operator=(const IGame &) -> IGame & = delete;

    IGame(IGame &&) = delete;
    auto operator=(IGame &&) -> IGame & = delete;

  protected:
    IGame() = default;
};
} // namespace atlas::core
