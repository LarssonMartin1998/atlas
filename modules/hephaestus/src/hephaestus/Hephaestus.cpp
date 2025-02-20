#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"

#include <print>

namespace atlas::hephaestus {
Hephaestus::Hephaestus(core::IEngine& engine) : core::Module{engine} {
    std::println("Hephaestus Constructor");
}

auto Hephaestus::start() -> void { std::println("Hephaestus::start()"); }
auto Hephaestus::shutdown() -> void { std::println("Hephaestus::shutdown()"); }

auto Hephaestus::tick() -> void {
    const auto& engine_ref = get_engine();
    for (auto& system : systems) {
        system->execute(engine_ref);
    }
}
auto Hephaestus::get_tick_rate() const -> unsigned { return 1; }

auto Hephaestus::generate_unique_entity_id() -> Entity {
    static Entity next_entity_id = 0;
    return next_entity_id++;
}
} // namespace atlas::hephaestus
