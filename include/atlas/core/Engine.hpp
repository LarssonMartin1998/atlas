#pragma once

#include <cassert>
#include <memory>
#include <print>
#include <unordered_map>

#include "core/Concepts.hpp"
#include "core/IEngine.hpp"
#include "core/IModule.hpp"
#include "core/ITickable.hpp"
#include "core/ModulesFactory.hpp"
#include "core/time/EngineClock.hpp"

namespace atlas::core {
template <TypeOfGame G> class Engine final : public IEngine {
  public:
    Engine() = default;
    ~Engine() override;

    Engine(const Engine&) = delete;
    auto operator=(const Engine&) -> Engine& = delete;

    Engine(Engine&&) = delete;
    auto operator=(Engine&&) -> Engine& = delete;

    auto run() -> void override;

    [[nodiscard]] auto get_game() -> IGame& override;

    [[nodiscard]] auto get_clock() const -> const IEngineClock& override;

  protected:
    [[nodiscard]] auto get_module_impl(std::type_index module) const
        -> IModule* override;

  private:
    auto tick_root() -> void;

    G game;

    std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules;
    std::vector<ITickable*> ticking_modules;

    EngineClock clock;
};

template <TypeOfGame G> Engine<G>::~Engine() {
    game.shutdown();

    for (auto& [module_type, module] : modules) {
        module->shutdown();
    }

    std::println("Engine destroyed");
}

template <TypeOfGame G> auto Engine<G>::run() -> void {
    std::println("Engine::run()");

    create_modules(*this, modules, ticking_modules);
    for (auto& [module_type, module] : modules) {
        module->start();
    }

    game.set_engine(*this);
    game.start();

    while (!game.should_quit()) {
        tick_root();

        clock.update_delta_time();
    }
}

template <TypeOfGame G> auto Engine<G>::get_game() -> IGame& { return game; }

template <TypeOfGame G>
auto Engine<G>::get_module_impl(std::type_index module) const -> IModule* {
    assert(modules.contains(module));
    const auto& module_ptr = modules.at(module);
    return module_ptr.get();
}

template <TypeOfGame G>
auto Engine<G>::get_clock() const -> const IEngineClock& {
    return clock;
}

template <TypeOfGame G> auto Engine<G>::tick_root() -> void {
    for (auto* module : ticking_modules) {
        module->tick();
    }
}
} // namespace atlas::core
