#pragma once

#include "core/Module.hpp"
#include "hephaestus/IHephaestus.hpp"

namespace atlas::hephaestus {
class Hephaestus final : public IHephaestus, public core::Module {
  public:
    explicit Hephaestus(core::IEngine &engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;
  protected:
    [[nodiscard]] auto generate_unique_entity_id() -> Entity override;
};
} // namespace atlas::hephaestus
