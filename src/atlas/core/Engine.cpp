#include "Engine.hpp"

#include <cassert>
#include <print>

#ifdef HEPHAESTUS
#include "Hephaestus.hpp"
#endif

namespace atlas::core {
Engine::Engine(std::unique_ptr<IGame> game) : game(std::move(game)) {
    std::println("Engine created");

    create_modules();

    for (auto &[module_type, module] : modules) {
        module->start();
    }
}

Engine::~Engine() {
    std::println("Engine destroyed");

    for (auto &[module_type, module] : modules) {
        module->shutdown();
    }
}

auto Engine::run() -> void {
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

auto Engine::create_modules() -> void {

#define MODULE(ModuleClassName, module_namespace_name)                         \
    std::unique_ptr<IModule> module =                                          \
        std::make_unique<module_namespace_name::ModuleClassName>(*this);       \
    if (auto *tickable = dynamic_cast<ITickable *>(module.get())) {            \
        ticking_modules.push_back(tickable);                                   \
        std::println("ModuleClassName module is tickable");                    \
    }                                                                          \
    modules.insert({EModules ::ModuleClassName, std ::move(module)});

#include "ModulesList.def"

#undef MODULE
}

auto Engine::tick_root() -> void {
    for (auto *module : ticking_modules) {
        module->tick();
    }
}
} // namespace atlas::core
