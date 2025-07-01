#pragma once

#include <ranges>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {

template <AllTypeOfComponent... ComponentTypes>
auto filter_archetypes(const ArchetypeMap& map) {
    // Create ArchetypeKey directly from template parameters
    const auto query_key = make_archetype_key<ComponentTypes...>();

    return map | std::ranges::views::filter([query_key](const auto& pair) {
               const auto& archetype_key = pair.first;
               // Check if the query key is a subset of the archetype key
               return query_key.is_subset_of(archetype_key);
           });
}

template <AllTypeOfComponent... ComponentTypes>
auto build_pipeline(const ArchetypeMap& map) {
    return filter_archetypes<ComponentTypes...>(map)
           | std::ranges::views::transform([&](auto const& pair) {
                 auto& archetype = *pair.second;
                 return archetype.template get_entity_tuples<ComponentTypes...>();
             })
           | std::ranges::views::join;
}
} // namespace atlas::hephaestus
