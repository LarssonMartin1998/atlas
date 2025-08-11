#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <ranges>
#include <taskflow/algorithm/for_each.hpp>
#include <taskflow/taskflow.hpp>
#include <tuple>
#include <utility>
#include <vector>

#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/query/Query.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

namespace atlas::core {
class IEngine;
}

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class System final : public SystemBase {
  public:
    using SystemFunc = std::function<void(const core::IEngine&, std::tuple<ComponentTypes&...>)>;

    explicit System(SystemFunc func)
        : func{std::move(func)} {}

    System(const System&) = delete;
    auto operator=(const System&) -> System& = delete;

    System(System&&) = delete;
    auto operator=(System&&) -> System& = delete;

    ~System() override = default;

    auto set_concurrent_systems(std::size_t estimate) -> void override;
    auto execute(const core::IEngine& engine, tf::Subflow& subflow) -> void override;
    auto cache_affected_archetypes(const ArchetypeMap& archetypes) -> void override;
    auto create_query() -> void override;

  private:
    auto calculate_archetype_version_cumsum() -> std::uint64_t;

    Query<ComponentTypes...> query;
    std::vector<std::reference_wrapper<Archetype>> affected_archetypes;
    SystemFunc func;

    // How many systems which are being executed
    // concurrently. This is estimated from the dependency
    // graph in hephaestus and used to dynamically adjust
    // the chunk size for parallel execution.
    std::size_t concurrent_systems_estimate = 1;
};

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::set_concurrent_systems(std::size_t estimate) -> void {
    concurrent_systems_estimate = estimate;
}

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::execute(const core::IEngine& engine, tf::Subflow& subflow) -> void {
    const auto& entity_components = query.get(calculate_archetype_version_cumsum());
    const auto entity_count = entity_components.size();

    if (entity_count == 0) {
        return;
    }

    constexpr std::size_t MIN_PARALLEL_THRESHOLD = 128;
    if (entity_count < MIN_PARALLEL_THRESHOLD) {
        for (const auto& data : entity_components) {
            func(engine, data);
        }
        return;
    }

    const auto num_workers = subflow.executor().num_workers();
    const auto effective_workers = std::max<std::size_t>(
        1,
        num_workers / concurrent_systems_estimate
    );

    constexpr std::size_t MIN_PARALLEL_WORKERS = MIN_PARALLEL_THRESHOLD / 2;
    auto chunk_size = std::max<std::size_t>(1, entity_count / effective_workers);
    chunk_size = std::max<std::size_t>(chunk_size, MIN_PARALLEL_WORKERS);

    subflow.for_each_index(
        std::size_t{0},
        entity_count,
        chunk_size,
        [this, &engine, &entity_components](std::size_t i) {
            func(engine, entity_components[i]);
        }
    );
}

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::cache_affected_archetypes(const ArchetypeMap& archetypes) -> void {
    affected_archetypes = filter_archetypes<ComponentTypes...>(archetypes)
                          | std::ranges::views::transform(
                              [&](const auto& pair) -> std::reference_wrapper<Archetype> {
                                  auto& archetype = *pair.second;
                                  return std::ref(archetype);
                              }
                          )
                          | std::ranges::to<std::vector>();
}

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::create_query() -> void {
    query.set_archetypes(affected_archetypes);
}

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::calculate_archetype_version_cumsum() -> std::uint64_t {
    return std::ranges::fold_left(
        affected_archetypes
            | std::views::transform([](const Archetype& archetype) -> std::uint64_t {
                  return archetype.get_version();
              }),
        std::uint64_t{0},
        std::plus<>{}
    );
}
} // namespace atlas::hephaestus
