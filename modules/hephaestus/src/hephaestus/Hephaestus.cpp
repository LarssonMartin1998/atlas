#include "hephaestus/Hephaestus.hpp"

#include <print>

namespace atlas::hephaestus {
Hephaestus::Hephaestus(core::IEngine &engine) : core::Module{engine} {
    std::println("Hephaestus Constructor");
}

auto Hephaestus::start() -> void { std::println("Hephaestus::start()"); }
auto Hephaestus::shutdown() -> void { std::println("Hephaestus::shutdown()"); }

auto Hephaestus::tick() -> void { std::println("Hephaestus::tick()"); }
auto Hephaestus::get_tick_rate() const -> unsigned { return 1; }
} // namespace atlas::hephaestus
