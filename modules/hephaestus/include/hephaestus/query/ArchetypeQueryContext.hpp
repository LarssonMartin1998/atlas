#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include <iostream>

namespace atlas::hephaestus {
struct ArchetypeQueryContext final {
    explicit ArchetypeQueryContext(
        const ArchetypeMap& archetypes,
        std::vector<std::type_index>&& component_types
    )
        : archetypes{archetypes}
        , component_types{std::move(component_types)} {

        std::cout << "QueryContext Constructor\n";
    }

    ArchetypeQueryContext(const ArchetypeQueryContext&) = delete;
    auto operator=(const ArchetypeQueryContext&) -> ArchetypeQueryContext& = delete;

    ArchetypeQueryContext(ArchetypeQueryContext&&) = delete;
    auto operator=(ArchetypeQueryContext&&) -> ArchetypeQueryContext& = delete;

    ~ArchetypeQueryContext() = default;

    const ArchetypeMap& archetypes;
    const std::vector<std::type_index> component_types;
};
} // namespace atlas::hephaestus
