#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class Query final {
  public:
    Query() = default;

    Query(const Query&) = delete;
    auto operator=(const Query&) = delete;

    Query(const Query&&) = delete;
    auto operator=(const Query&&) = delete;

    ~Query() = default;

  private:
    using ComponentsVector = std::vector<std::tuple<ComponentTypes&...>>;

  public:
    [[nodiscard]]
    inline auto get(std::uint64_t version_cumsum) const -> ComponentsVector&;

    inline auto set_archetypes(std::span<std::reference_wrapper<Archetype>> archetypes) -> void;

  private:
    [[nodiscard]] inline auto is_cache_dirty(std::uint64_t version_cumsum) const -> bool;

    mutable std::optional<ComponentsVector> cache;
    mutable std::uint64_t last_version_cumsum = 0;
    std::span<std::reference_wrapper<Archetype>> archetypes;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::is_cache_dirty(
    const std::uint64_t version_cumsum
) const -> bool {
    if (!cache.has_value()) {
        return true;
    }

    if (last_version_cumsum != version_cumsum) {
        return true;
    }

    return false;
}

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get(const std::uint64_t version_cumsum) const
    -> ComponentsVector& {
    if (is_cache_dirty(version_cumsum)) {
        last_version_cumsum = version_cumsum;
        auto pipeline = build_pipeline<ComponentTypes...>(archetypes);

        // We evaluate the pipeline and collect it into a vector.
        // This costs one iteration over the data, but enables size storage and
        // random access. This can be used to chunk and parellize the execution
        // of the systems. And should result in better performance and
        // utilization.
        ComponentsVector comp_vec = std::ranges::to<std::vector>(pipeline);
        cache.emplace(std::move(comp_vec));
    }

    return *cache;
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::set_archetypes(
    std::span<std::reference_wrapper<Archetype>> new_archetypes
) -> void {
    archetypes = new_archetypes;
}
} // namespace atlas::hephaestus
