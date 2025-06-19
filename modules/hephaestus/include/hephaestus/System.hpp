#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/query/Query.hpp"
#include <algorithm>
#include <functional>
#include <ranges>
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
                                          std::tuple<ComponentTypes&...>)>;

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
        for (auto data : query.get()) {
            func(engine, data);
        }
        // auto components = query.get();
        // constexpr auto available_threads = 5;
        //
        // auto chunks = components | std::views::chunk(available_threads);
        // for (const auto& chunk : chunks) {
        //     for (auto component : chunk) {
        //         std::print("TJEEEENA");
        //         func(engine, component);
        //     }
        // }
        // auto chunks = components | std::views::enumerate |
        //               std::views::chunk_by([&](const auto& a, const auto& b)
        //               {
        //                   return a.first / (available_threads) ==
        //                          b.first / (available_threads);
        //               });
        // tf::Taskflow taskflow;
        // for (const auto& chunk : chunks) {
        //     taskflow.emplace([this, chunk, &engine] {
        //         for (const auto& [_, component] : chunk) {
        //             func(engine, component);
        //         }
        //     });
        // }
        //
        // tf::Executor executor;
        // executor.run(taskflow).wait();
    }

  private:
    Query<ComponentTypes...> query;
    SystemFunc func;
};
} // namespace atlas::hephaestus
