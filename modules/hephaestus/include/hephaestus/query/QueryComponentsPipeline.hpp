#pragma once

#include <algorithm>
#include <ranges>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {

template <AllTypeOfComponent... ComponentTypes>
auto filter_archetypes_to_signature(
    const ArchetypeMap& map,
    const std::vector<ComponentAccess>& signature
) {
    return map | std::ranges::views::filter([&](const auto& pair) {
               const auto& archetype_component_types = pair.first;
               return std::ranges::all_of(
                   signature,
                   [&archetype_component_types](const auto& component_access) {
                       return std::ranges::find(archetype_component_types, component_access.type)
                              != archetype_component_types.end();
                   }
               );
           });
}

template <AllTypeOfComponent... ComponentTypes>
auto build_pipeline(const ArchetypeMap& map, const std::vector<ComponentAccess>& signature) {
    return filter_archetypes_to_signature(map, signature)
           | std::ranges::views::transform([&](auto const& pair) {
                 auto& archetype = *pair.second;
                 return archetype.template get_entity_tuples<ComponentTypes...>();
             })
           | std::ranges::views::join;
}
} // namespace atlas::hephaestus
