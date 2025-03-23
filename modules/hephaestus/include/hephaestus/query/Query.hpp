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
          std::vector<std::type_index> component_types)
        : context{archetypes,
                  {component_types.begin(), component_types.end()}} {
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
        std::declval<const std::unordered_set<std::type_index>&>()));

  public:
    [[nodiscard]]
    inline auto get() const -> QueryResult<PipelineType>&;

  private:
    mutable std::optional<QueryResult<PipelineType>> cache;
    const ArchetypeQueryContext context;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get() const
    -> QueryResult<PipelineType>& {
    if (!cache.has_value()) {
        cache.emplace(build_pipeline<ComponentTypes...>(context.archetypes,
                                                        context.signature));
    }

    return *cache;
}
} // namespace atlas::hephaestus
