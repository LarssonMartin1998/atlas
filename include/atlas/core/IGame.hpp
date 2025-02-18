#pragma once

#include "core/IEngineHandle.hpp"
#include "core/IStartShutdown.hpp"

namespace atlas::core {
class IGame : public IStartShutdown, public IEngineHandle {
  public:
    ~IGame() override = default;

    IGame(const IGame&) = delete;
    auto operator=(const IGame&) -> IGame& = delete;

    IGame(IGame&&) = delete;
    auto operator=(IGame&&) -> IGame& = delete;

    virtual auto set_engine(IEngine& engine_ref) -> void = 0;

    [[nodiscard]] virtual auto should_quit() const -> bool = 0;

  protected:
    IGame() = default;
};
} // namespace atlas::core
