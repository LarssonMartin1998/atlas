#pragma once

#include <optional>
#include <print>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/query/ArchetypeQueryContext.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes> class Query final {
  public:
    Query(const ArchetypeMap& archetypes,
          std::vector<std::type_index>&& component_types)
        : context{archetypes, std::move(component_types)} {
        std::println("Query Constructor");
    }

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
    [[nodiscard]] inline auto
    is_cache_dirty(const std::uint64_t& cumsum_version) const -> bool;
    [[nodiscard]] inline auto calc_components_cumsum_version() const
        -> std::uint64_t;

    mutable std::uint64_t last_cache_cumsum_version{};
    mutable std::optional<ComponentsVector> cache;

    const ArchetypeQueryContext context;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::is_cache_dirty(
    const std::uint64_t& cumsum_version) const -> bool {

    if (!cache.has_value()) {
        return true;
    }

    if (cumsum_version != last_cache_cumsum_version) {
        return true;
    }

    return false;
}

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto
Query<ComponentTypes...>::calc_components_cumsum_version() const
    -> std::uint64_t {
    return (0ULL + ... + Component<ComponentTypes>::get_version());
}

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get() const
    -> ComponentsVector& {
    const auto cumsum_version = calc_components_cumsum_version();
    if (is_cache_dirty(cumsum_version)) {
        auto pipeline = build_pipeline<ComponentTypes...>(
            context.archetypes, context.component_types);

        // We evaluate the pipeline and collect it into a vector.
        // This costs one iteration over the data, but enables size storage and
        // random access. This can be used to chink and parellize the execution
        // of the systems. And should result in better performance and
        // utilization.
        ComponentsVector comp_vec = std::ranges::to<std::vector>(pipeline);
        cache.emplace(std::move(comp_vec));
        last_cache_cumsum_version = cumsum_version;
    }

    return *cache;
}
} // namespace atlas::hephaestus
