#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"
#include "core/threads/IThreadPool.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

#include <algorithm>
#include <cstddef>
#include <print>
// #include <taskflow/taskflow.hpp>

namespace atlas::hephaestus {
Hephaestus::Hephaestus(core::IEngine& engine) : core::Module{engine} {
    constexpr auto queue_buffert = 100;
    creation_queue.reserve(queue_buffert);
    destroy_queue.reserve(queue_buffert);
    std::println("Hephaestus Constructor");
}

auto Hephaestus::start() -> void { std::println("Hephaestus::start()"); }

// Create all dendency graphs and taskflow shit here
auto Hephaestus::post_start() -> void {
    build_systems_dependency_graph();
    // tf::Taskflow tf;
    // auto result = tf.emplace([]() {
    //     std::println("");
    //     return 1;
    // });
}

auto Hephaestus::shutdown() -> void { std::println("Hephaestus::shutdown()"); }

auto Hephaestus::tick() -> void {
    for (auto& creation : creation_queue) {
        creation();
    }
    creation_queue.clear();

    auto& engine = get_engine();
    auto& thread_pool = engine.get_thread_pool();

    for (auto& system : systems) {
        thread_pool.enqueue(
            [this, &system, &engine]() { system->execute(engine); });
    }
    thread_pool.await_all_tasks_completed();

    //
    // Destroy queued entities
}
auto Hephaestus::get_tick_rate() const -> unsigned { return 1; }

auto Hephaestus::generate_unique_entity_id() -> Entity {
    static Entity next_entity_id = 0;
    return next_entity_id++;
}

auto Hephaestus::build_systems_dependency_graph() -> void {
    assert(
        system_nodes != std::nullopt &&
        "system_nodes has been reset before the dependency_graph was built.");

    std::vector<std::vector<std::size_t>> system_deps = {};

    const auto num_nodes = (*system_nodes).size();
    system_deps.reserve(num_nodes);

    for (auto& node : *system_nodes) {
        auto filtered = filter_archetypes_to_signature(
            archetypes, node.component_dependencies);
        node.affected_archetypes.reserve(archetypes.size());
        for (const auto& [idx, _] : filtered) {
            node.affected_archetypes.emplace_back(std::ref(idx));
        }
    }

    size_t covered = 0;
    for (size_t i = 0; i < num_nodes; i++) {
        auto& node = (*system_nodes)[i];

        std::vector<std::size_t> tmp;
        tmp.reserve(num_nodes);
        auto& node_deps = system_deps.emplace_back(std::move(tmp));
        node_deps.reserve(num_nodes);

        for (size_t j = covered; j < num_nodes; j++) {
            auto& other = (*system_nodes)[j];

            if (&node == &other) {
                continue;
            }

            if (are_signatures_overlapping(node.component_dependencies,
                                           other.component_dependencies)) {
                node_deps.emplace_back(j);
                continue;
            }

            if ([&node, &other]() {
                    for (const auto& idx : node.affected_archetypes) {
                        for (const auto& other_idx :
                             other.affected_archetypes) {
                            if (are_signatures_overlapping(idx.get(),
                                                           other_idx.get())) {
                                return true;
                            }
                        }
                    }
                    return false;
                }()) {
                node_deps.emplace_back(j);
                continue;
            }
        }

        covered++;
    }

    // todo: create a taskflow graph from the system_deps

    system_nodes.reset();
}
} // namespace atlas::hephaestus
