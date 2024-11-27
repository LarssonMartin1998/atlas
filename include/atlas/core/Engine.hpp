#pragma once

#include <memory>
#include <unordered_map>

#include "EModules.hpp"
#include "IEngine.hpp"
#include "IGame.hpp"
#include "IModule.hpp"
#include "ITickable.hpp"

namespace atlas::core {
class Engine final : public IEngine {
  public:
    explicit Engine(std::unique_ptr<IGame> game);
    ~Engine() override;

    Engine(const Engine &) = delete;
    auto operator=(const Engine &) -> Engine & = delete;

    Engine(Engine &&) = delete;
    auto operator=(Engine &&) -> Engine & = delete;

    auto run() -> void override;

    [[nodiscard]] auto
    get_game() const -> std::reference_wrapper<IGame> override;

  protected:
    [[nodiscard]] auto
    get_module_impl(EModules module) const -> IModule * override;

  private:
    // auto create_modules() -> void;

    auto tick_root() -> void;

    std::unordered_map<EModules, std::unique_ptr<IModule>> modules;
    std::vector<ITickable *> ticking_modules;
    std::unique_ptr<IGame> game;
};
} // namespace atlas::core
