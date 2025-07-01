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
    // Convert ComponentAccess vector to ComponentSignature for comparison
    const auto query_signature = component_access_to_signature(signature);
    
    return map | std::ranges::views::filter([query_signature](const auto& pair) {
               const auto& archetype_signature = pair.first;
               // Check if the query signature is a subset of the archetype signature
               return query_signature.is_subset_of(archetype_signature);
           });
}

template <AllTypeOfComponent... ComponentTypes>
auto build_pipeline(const ArchetypeMap& map, const std::vector<ComponentAccess>& signature) {
    return filter_archetypes_to_signature<ComponentTypes...>(map, signature)
           | std::ranges::views::transform([&](auto const& pair) {
                 auto& archetype = *pair.second;
                 return archetype.template get_entity_tuples<ComponentTypes...>();
             })
           | std::ranges::views::join;
}
} // namespace atlas::hephaestus
