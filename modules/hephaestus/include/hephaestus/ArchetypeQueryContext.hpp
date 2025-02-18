#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include <print>
#include <unordered_set>

namespace atlas::hephaestus {
struct ArchetypeQueryContext final {
    explicit ArchetypeQueryContext(
        const ArchetypeMap& archetypes,
        std::unordered_set<std::type_index>&& signature)
        : archetypes{archetypes}, signature{signature} {

        std::println("QueryContext Constructor");
    }

    ArchetypeQueryContext(const ArchetypeQueryContext&) = delete;
    auto operator=(const ArchetypeQueryContext&)
        -> ArchetypeQueryContext& = delete;

    ArchetypeQueryContext(ArchetypeQueryContext&&) = delete;
    auto operator=(ArchetypeQueryContext&&) -> ArchetypeQueryContext& = delete;

    ~ArchetypeQueryContext() = default;

    const ArchetypeMap& archetypes;
    const std::unordered_set<std::type_index> signature;
};
} // namespace atlas::hephaestus
