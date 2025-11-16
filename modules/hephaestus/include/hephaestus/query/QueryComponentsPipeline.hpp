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
} // namespace atlas::hephaestus
