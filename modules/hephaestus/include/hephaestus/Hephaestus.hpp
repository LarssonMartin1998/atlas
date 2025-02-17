#pragma once

#include "core/Module.hpp"
#include "hephaestus/IHephaestus.hpp"

namespace atlas::hephaestus {
class Hephaestus final : public IHephaestus, public core::Module {
  public:
    explicit Hephaestus(core::IEngine& engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;
};
} // namespace atlas::hephaestus
