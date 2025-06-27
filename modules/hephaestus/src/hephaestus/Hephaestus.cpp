#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

#include <cstddef>
#include <functional>
#include <print>

namespace atlas::hephaestus {
Hephaestus::Hephaestus(core::IEngine& engine)
    : core::Module{engine}
    ,
    // Might want to reserve some threads for other tasks such as rendering,
    // physics and other stuff.
    // This is OK for now, but we should handle this in a centralized way
    // later on to make sure that we don't have too many threads.
    systems_executor(std::thread::hardware_concurrency()) {
    constexpr auto QUEUE_BUFFERT = 100;
    creation_queue.reserve(QUEUE_BUFFERT);
    destroy_queue.reserve(QUEUE_BUFFERT);
    std::println("Hephaestus Constructor");
}

auto Hephaestus::start() -> void {
    std::println("Hephaestus::start()");
}

auto Hephaestus::post_start() -> void {
    build_systems_dependency_graph();
}

auto Hephaestus::shutdown() -> void {
    std::println("Hephaestus::shutdown()");
}

auto Hephaestus::tick() -> void {
    for (auto& creation : creation_queue) {
        creation();
    }
    creation_queue.clear();

    if (!systems_graph.empty()) {
        systems_executor.run(systems_graph).wait();
    }

    //
    // TODO:
    // Destroy queued entities
}
auto Hephaestus::get_tick_rate() const -> unsigned {
    return 1;
}

auto Hephaestus::generate_unique_entity_id() -> Entity {
    static Entity next_entity_id = 0;
    return next_entity_id++;
}

auto Hephaestus::build_systems_dependency_graph() -> void {
    assert(
        system_nodes != std::nullopt
        && "system_nodes has been reset before the dependency_graph was built."
    );

    const auto num_nodes = (*system_nodes).size();
    if (num_nodes == 0) {
        return;
    }

    std::vector<std::vector<std::size_t>> system_deps(num_nodes);
    // Very pessimistic guesswork for inner vector capacity, but safe.
    // Choose a more realistic number if needed.
    for (size_t i = 0; i < num_nodes; ++i) {
        system_deps[i].reserve(num_nodes);
    }

    for (auto& node : *system_nodes) {
        auto filtered = filter_archetypes_to_signature(archetypes, node.component_dependencies);
        node.affected_archetypes.reserve(archetypes.size());
        for (const auto& [idx, _] : filtered) {
            node.affected_archetypes.emplace_back(std::ref(idx));
        }
    }

    const auto are_nodes_conflicting = [](const SystemNode& node, const SystemNode& other) {
        // Use const-aware access signature conflict detection
        if (are_access_signatures_overlapping(node.component_access_dependencies, other.component_access_dependencies)) {
            return true;
        }

        for (const auto& i : node.affected_archetypes) {
            for (const auto& j : other.affected_archetypes) {
                if (are_signatures_overlapping(i.get(), j.get())) {
                    return true;
                }
            }
        }

        return false;
    };

    for (size_t i = 0; i < num_nodes; ++i) {
        auto& node = (*system_nodes)[i];

        for (size_t j = i + 1; j < num_nodes; ++j) {
            auto& other = (*system_nodes)[j];

            if (are_nodes_conflicting(node, other)) {
                system_deps[i].emplace_back(j);
                system_deps[j].emplace_back(i);
            }
        }
    }

    for (std::size_t i = 0; i < num_nodes; ++i) {
        const auto conflicts = system_deps[i].size();
        const auto concurrent = std::max<std::size_t>(1, num_nodes - conflicts);
        systems[i]->set_concurrent_systems(concurrent);
    }

    std::vector<tf::Task> tasks(num_nodes);
    for (size_t i = 0; i < num_nodes; ++i) {
        tasks[i] = systems_graph.emplace([this, i](tf::Subflow& subflow) {
            systems[i]->execute(get_engine(), subflow);
        });
    }

    for (size_t i = 0; i < num_nodes; ++i) {
        for (size_t j : system_deps[i]) {
            if (i < j) {
                tasks[i].precede(tasks[j]);
            }
        }
    }
}
} // namespace atlas::hephaestus
