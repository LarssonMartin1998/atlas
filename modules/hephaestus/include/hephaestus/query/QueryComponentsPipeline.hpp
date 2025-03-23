#pragma once

#include <algorithm>
#include <ranges>
#include <typeindex>
#include <unordered_set>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {

template <AllTypeOfComponent... ComponentTypes>
auto build_pipeline(const ArchetypeMap& map,
                    const std::unordered_set<std::type_index>& signature) {
    return map | std::ranges::views::filter([&](auto const& pair) {
               const auto& archetype_component_types = pair.first;
               return std::ranges::all_of(
                   signature,
                   [&archetype_component_types](const auto& signature_type) {
                       return std::ranges::find(archetype_component_types,
                                                signature_type) !=
                              archetype_component_types.end();
                   });
           }) |
           std::ranges::views::transform([&](auto const& pair) {
               auto& archetype = *pair.second;
               return archetype.template get_entity_tuples<ComponentTypes...>();
           }) |
           std::ranges::views::join;
}

} // namespace atlas::hephaestus
