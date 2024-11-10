#include "Engine.hpp"

#include <iostream>

namespace atlas::core {
Engine::Engine(std::unique_ptr<IGame> game) : game(std::move(game)) {
    std::cout << "Engine created\n";
}

auto Engine::get_module_impl(EModules module) const
    -> std::optional<IModule *> {
    const auto &module_ptr = modules.at(static_cast<size_t>(module));
    if (!module_ptr) {
        return std::nullopt;
    }

    return module_ptr.get();
}
} // namespace atlas::core
