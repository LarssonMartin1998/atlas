#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Utils.hpp"
#include "atlas/common/print_compat.hpp"

namespace atlas::hephaestus {
struct ArchetypeQueryContext final {
    explicit ArchetypeQueryContext(
        const ArchetypeMap& archetypes,
        std::vector<ComponentAccess> component_types
    )
        : archetypes{archetypes}
        , component_types{std::move(component_types)} {

        std::println("QueryContext Constructor");
    }

    ArchetypeQueryContext(const ArchetypeQueryContext&) = delete;
    auto operator=(const ArchetypeQueryContext&) -> ArchetypeQueryContext& = delete;

    ArchetypeQueryContext(ArchetypeQueryContext&&) = delete;
    auto operator=(ArchetypeQueryContext&&) -> ArchetypeQueryContext& = delete;

    ~ArchetypeQueryContext() = default;

    const ArchetypeMap& archetypes;
    const std::vector<ComponentAccess> component_types;
};
} // namespace atlas::hephaestus
