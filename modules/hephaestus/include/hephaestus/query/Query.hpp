#pragma once

#include <optional>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/query/ArchetypeQueryContext.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class Query final {
  public:
    Query(const ArchetypeMap& archetypes, std::vector<SystemDependencies> dependencies)
        : context{archetypes, std::move(dependencies)} {}

    Query(const Query&) = delete;
    auto operator=(const Query&) = delete;

    Query(const Query&&) = delete;
    auto operator=(const Query&&) = delete;

    ~Query() = default;

  private:
    using ComponentsVector = std::vector<std::tuple<ComponentTypes&...>>;

  public:
    [[nodiscard]]
    inline auto get() const -> ComponentsVector&;

  private:
    [[nodiscard]] inline auto is_cache_dirty() const -> bool;

    mutable std::optional<ComponentsVector> cache;

    const ArchetypeQueryContext context;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::is_cache_dirty() const -> bool {
    return !cache.has_value();
}

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get() const -> ComponentsVector& {
    if (is_cache_dirty()) {
        auto pipeline = build_pipeline<ComponentTypes...>(context.archetypes);

        // We evaluate the pipeline and collect it into a vector.
        // This costs one iteration over the data, but enables size storage and
        // random access. This can be used to chink and parellize the execution
        // of the systems. And should result in better performance and
        // utilization.
        ComponentsVector comp_vec = std::ranges::to<std::vector>(pipeline);
        cache.emplace(std::move(comp_vec));
    }

    return *cache;
}
} // namespace atlas::hephaestus
