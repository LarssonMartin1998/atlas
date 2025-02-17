#pragma once

#include "core/IEngineHandle.hpp"
#include "core/IStartShutdown.hpp"
#include <memory>

namespace atlas::core {
class IGame : public IStartShutdown, public IEngineHandle {
  public:
    ~IGame() override = default;

    IGame(const IGame&) = delete;
    auto operator=(const IGame&) -> IGame& = delete;

    IGame(IGame&&) = delete;
    auto operator=(IGame&&) -> IGame& = delete;

    [[nodiscard]] virtual auto should_quit() const -> bool = 0;

  protected:
    IGame() = default;

    friend class Engine;
    virtual auto set_engine(std::weak_ptr<IEngine> engine_ptr) -> void = 0;
};
} // namespace atlas::core
