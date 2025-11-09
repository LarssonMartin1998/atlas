#pragma once

#include <algorithm>
#include <functional>
#include <taskflow/algorithm/for_each.hpp>
#include <taskflow/taskflow.hpp>
#include <tuple>
#include <utility>

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/query/Query.hpp"

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
    auto create_query(const ArchetypeMap& archetypes) -> void override;

  private:
    Query<ComponentTypes...> query;
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
    const auto& components_cache_buckets = query.get();
    for (const auto& cache_bucket : components_cache_buckets) {
        const auto& components = cache_bucket.components;
        const auto entity_count = components.size();

        // READ COMMENT BELOW FOR EXPLANATION REGARDING THIS CODE
        // ------------------------------------------------------
        //             constexpr std::size_t MIN_PARALLEL_THRESHOLD = 128;
        //             if (entity_count < MIN_PARALLEL_THRESHOLD) {
        //             for (const auto& data : components) {
        //                 func(engine, data);
        //             }
        //                 continue;
        //             }
        //             const auto num_workers = subflow.executor().num_workers();
        //             const auto effective_workers = std::max<std::size_t>(
        //                 1,
        //                 num_workers / concurrent_systems_estimate
        //             );
        //             constexpr std::size_t MIN_CHUNK_SIZE = MIN_PARALLEL_THRESHOLD / 2;
        //             auto chunk_size = std::max<std::size_t>(1, entity_count / effective_workers);
        //             chunk_size = std::max<std::size_t>(chunk_size, MIN_CHUNK_SIZE);
        // ------------------------------------------------------

        // I cant explain this, but when using a subflow to iterate each bucket, the cpu utilization
        // is drastically improved. Additionally, the scheduling is better in ALL testing I have
        // done when using the entity_count as the chunk size, you would think that this would be
        // the same as simply iterating the bucket regularly within the original taskflow, however,
        // them the cpu utilization is crippled across all cores to ~35% condistently on
        // windows/mac/linux, tested on multiple computers.
        // This even improves the frame time for when we have cache rebuilds ¯\_(ツ)_/¯
        //
        // This is currently the best result across multiple configurations, however, I am keeping
        // my original implementation above commented out until I understand this or can find a
        // logical reason for why this is happening.
        subflow.for_each_index(
            std::size_t{0},
            entity_count,
            std::max<std::size_t>(entity_count, 1),
            [this, &engine, &components](std::size_t i) {
                func(engine, components[i]);
            }
        );
    }
}

template <AllTypeOfComponent... ComponentTypes>
auto System<ComponentTypes...>::create_query(const ArchetypeMap& archetypes) -> void {
    query.set_archetypes(archetypes);
}
} // namespace atlas::hephaestus
