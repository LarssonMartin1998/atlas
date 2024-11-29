#pragma once

#include <typeindex>
#include <vector>

#include "hephaestus/ArchetypeBase.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class Archetype final : public ArchetypeBase {
  public:
    using EntityComponents = std::tuple<Entity, ComponentTypes...>;

    Archetype()
        : component_types(make_component_type_signature<ComponentTypes...>()),
          ArchetypeBase() {}

    auto create_entity(Entity entity) -> void {
        entities.emplace_back(entity, ComponentTypes{}...);
    }

  private:
    std::vector<std::type_index> component_types;
    std::vector<EntityComponents> entities;
};
} // namespace atlas::hephaestus
