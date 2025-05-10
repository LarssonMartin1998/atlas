#pragma once

#include <optional>
#include <print>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/query/ArchetypeQueryContext.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"
#include "hephaestus/query/QueryResult.hpp"

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
    using PipelineType = decltype(build_pipeline<ComponentTypes...>(
        std::declval<const ArchetypeMap&>(),
        std::declval<const std::vector<std::type_index>&>()));

  public:
    [[nodiscard]]
    inline auto get() const -> QueryResult<PipelineType>&;

  private:
    [[nodiscard]] inline auto
    is_cache_dirty(const std::uint64_t& cumsum_version) const -> bool;
    [[nodiscard]] inline auto calc_components_cumsum_version() const
        -> std::uint64_t;

    mutable std::uint64_t last_cache_cumsum_version{};
    mutable std::optional<QueryResult<PipelineType>> cache;

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
    -> QueryResult<PipelineType>& {
    const auto cumsum_version = calc_components_cumsum_version();
    if (is_cache_dirty(cumsum_version)) {
        cache.emplace(build_pipeline<ComponentTypes...>(
            context.archetypes, context.component_types));
        last_cache_cumsum_version = cumsum_version;
    }

    return *cache;
}
} // namespace atlas::hephaestus
