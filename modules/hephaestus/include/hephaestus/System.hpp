#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/query/Query.hpp"
#include <functional>
#include <numeric>
#include <print>
#include <taskflow/taskflow.hpp>
#include <tuple>
#include <utility>

namespace atlas::core {
class IEngine;
}

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class System final : public SystemBase {
  public:
    using SystemFunc = std::function<void(const core::IEngine&,
                                          std::tuple<ComponentTypes...>&)>;

    explicit System(SystemFunc func, const ArchetypeMap& archetypes,
                    std::vector<std::type_index>&& component_types)
        : func{std::move(func)}, query{archetypes, std::move(component_types)} {
    }

    System(const System&) = delete;
    auto operator=(const System&) -> System& = delete;

    System(System&&) = delete;
    auto operator=(System&&) -> System& = delete;

    ~System() override = default;

    auto execute(const core::IEngine& engine) -> void override {
        // tf::Taskflow taskflow;
        // tf::Executor executor;
        // std::vector<int> data(100);
        // std::ranges::iota(std::begin(data), std::end(data), 0);
        // taskflow.for_each_task(
        //     std::begin(data), std::end(data),
        //     [](auto& element) { std::println("Task {}", element); });
        // auto query_range = query.get();
        // taskflow.for_each(std::begin(query.get()), std::end(query_range),
        //                   [this, &engine](std::tuple<ComponentTypes...>&
        //                   data) {
        //                       func(engine, data);
        //                   });
        // executor.run(taskflow).wait();
    }

  private:
    Query<ComponentTypes...> query;
    SystemFunc func;
};

} // namespace atlas::hephaestus
