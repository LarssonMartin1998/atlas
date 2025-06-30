#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/Utils.hpp"
#include "hephaestus/query/Query.hpp"
#include <algorithm>
#include <functional>
#include <taskflow/algorithm/for_each.hpp>
#include <taskflow/taskflow.hpp>
#include <tuple>
#include <utility>
#include <vector>

namespace atlas::core {
class IEngine;
}

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class System final : public SystemBase {
  public:
    using SystemFunc = std::function<void(const core::IEngine&, std::tuple<ComponentTypes&...>)>;

    explicit System(
        SystemFunc func,
        const ArchetypeMap& archetypes,
        std::vector<ComponentAccess> component_types
    )
        : func{std::move(func)}
        , query{archetypes, std::move(component_types)} {}

    System(const System&) = delete;
    auto operator=(const System&) -> System& = delete;

    System(System&&) = delete;
    auto operator=(System&&) -> System& = delete;

    ~System() override = default;

    auto set_concurrent_systems(std::size_t estimate) -> void override;
    auto execute(const core::IEngine& engine, tf::Subflow& subflow) -> void override;

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
    const auto& entity_components = query.get();
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
        0,
        entity_count,
        chunk_size,
        [this, &engine, &entity_components](std::size_t i) {
            func(engine, entity_components[i]);
        }
    );
}
} // namespace atlas::hephaestus
