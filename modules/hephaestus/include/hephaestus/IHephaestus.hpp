#pragma once

#include "core/IModule.hpp"
#include "core/ITickable.hpp"
#include "hephaestus/Common.hpp"

namespace atlas::hephaestus {
class IHephaestus : public virtual core::IModule, public core::ITickable {
  public:
    ~IHephaestus() override = default;

    IHephaestus(const IHephaestus &) = delete;
    auto operator=(const IHephaestus &) -> IHephaestus & = delete;

    IHephaestus(IHephaestus &&) = delete;
    auto operator=(IHephaestus &&) -> IHephaestus & = delete;

    auto create_entity() -> void;

  protected:
    IHephaestus() = default;

    [[nodiscard]] virtual auto generate_unique_entity_id() -> Entity = 0;
};
} // namespace atlas::hephaestus
