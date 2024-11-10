#pragma once

#include <array>
#include <memory>
#include <optional>

#include "EModules.hpp"
#include "IEngine.hpp"
#include "IGame.hpp"
#include "IModule.hpp"

namespace atlas::core {
class Engine final : public IEngine {
  public:
    explicit Engine(std::unique_ptr<IGame> game);

  protected:
    [[nodiscard]] auto
    get_module_impl(EModules module) const -> std::optional<IModule *> override;

  private:
    std::array<std::unique_ptr<IModule>,
               static_cast<size_t>(EModules::NumModules)>
        modules;
    std::unique_ptr<IGame> game;
};
} // namespace atlas::core
