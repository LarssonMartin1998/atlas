#pragma once

#include <functional>
#include <ranges>

#include "hephaestus/Archetype.hpp"
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
auto build_pipeline(std::span<std::reference_wrapper<Archetype>> archetypes) {
    return archetypes
           | std::ranges::views::transform([&](std::reference_wrapper<Archetype> archetype_ref) {
                 auto& archetype = archetype_ref.get();
                 return archetype.template get_entity_tuples<ComponentTypes...>();
             })
           | std::ranges::views::join;
}
} // namespace atlas::hephaestus
