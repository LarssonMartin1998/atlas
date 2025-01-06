#include "core/Engine.hpp"

#include <cassert>
#include <memory>
#include <print>

#include "core/IGame.hpp"
#include "core/ModulesFactory.hpp"

namespace atlas::core {
Engine::Engine(std::unique_ptr<IGame> game_ptr) : game(std::move(game_ptr)) {
    std::println("Engine created");
}

Engine::~Engine() {
    game->shutdown();

    for (auto &[module_type, module] : modules) {
        module->shutdown();
    }

    std::println("Engine destroyed");
}

auto Engine::run() -> void {
    std::println("Engine::run()");

    create_modules(*this, modules, ticking_modules);
    for (auto &[module_type, module] : modules) {
        module->start();
    }

    game->set_engine(shared_from_this());
    game->start();

    while (!game->should_quit()) {
        tick_root();
    }
}

auto Engine::get_game() const -> std::reference_wrapper<IGame> {
    assert(game);
    return std::ref(*game);
}

auto Engine::get_module_impl(EModules module) const -> IModule * {
    assert(modules.contains(module));
    const auto &module_ptr = modules.at(module);
    return module_ptr.get();
}

auto Engine::tick_root() -> void {
    for (auto *module : ticking_modules) {
        module->tick();
    }
}
} // namespace atlas::core
