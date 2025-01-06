#pragma once

#include <algorithm>
#include <vector>

#include "core/ITickable.hpp"
#include "core/Module.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/System.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
class Hephaestus;
}

namespace atlas::hephaestus {
class Hephaestus final : public core::Module, public core::ITickable {
  public:
    explicit Hephaestus(core::IEngine &engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;

    template <typename Func, AllTypeOfComponent... ComponentTypes>
    auto create_system(Func &&func) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity() -> void;

  protected:
    [[nodiscard]] static auto generate_unique_entity_id() -> Entity;

  private:
    std::vector<std::unique_ptr<SystemBase>> systems;
    ArchetypeMap archetypes;
};

template <typename Func, AllTypeOfComponent... ComponentTypes>
auto Hephaestus::create_system(Func &&func) -> void {
    auto new_system = std::make_unique<System<ComponentTypes...>>(
        std::forward<Func>(func), archetypes,
        make_component_type_signature<ComponentTypes...>());

    systems.emplace_back(std::move(new_system));
}

template <AllTypeOfComponent... ComponentTypes>
auto Hephaestus::create_entity() -> void {
    const auto entity_id = generate_unique_entity_id();
    const auto signature = make_component_type_signature<ComponentTypes...>();

    auto &archetype = [this, signature]() -> ArchetypePtr & {
        if (!archetypes.contains(signature)) {
            archetypes.emplace(signature, std::make_unique<Archetype>());
        }

        return archetypes[signature];
    }();

    archetype.get()->template create_entity<ComponentTypes...>(entity_id);
}
} // namespace atlas::hephaestus
