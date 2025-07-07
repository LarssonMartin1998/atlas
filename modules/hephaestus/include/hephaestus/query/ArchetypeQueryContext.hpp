#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
struct ArchetypeQueryContext final {
    explicit ArchetypeQueryContext(
        const ArchetypeMap& archetypes,
        std::vector<SystemDependencies> dependencies
    )
        : archetypes{archetypes}
        , dependencies{std::move(dependencies)} {}

    ArchetypeQueryContext(const ArchetypeQueryContext&) = delete;
    auto operator=(const ArchetypeQueryContext&) -> ArchetypeQueryContext& = delete;

    ArchetypeQueryContext(ArchetypeQueryContext&&) = delete;
    auto operator=(ArchetypeQueryContext&&) -> ArchetypeQueryContext& = delete;

    ~ArchetypeQueryContext() = default;

    const ArchetypeMap& archetypes;
    const std::vector<SystemDependencies> dependencies;
};
} // namespace atlas::hephaestus
