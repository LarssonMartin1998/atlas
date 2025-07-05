#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"

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

    constexpr auto ARCHETYPE_BUFFERT = 30;
    archetypes.reserve(ARCHETYPE_BUFFERT);
    ent_to_archetype_key.reserve(ARCHETYPE_BUFFERT);

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

    for (const auto entity : destroy_queue) {
        assert(
            ent_to_archetype_key.contains(entity)
            && "Trying to destroy an entity that doesnt exist!"
        );

        auto& archetype = *archetypes.at(ent_to_archetype_key.at(entity));
        if (archetype.destroy_entity(entity)) {
            ent_to_archetype_key.erase(entity);
        }
    }
    destroy_queue.clear();
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
    for (std::size_t i = 0; i < num_nodes; ++i) {
        system_deps[i].reserve(num_nodes);
    }

    for (auto& node : *system_nodes) {
        // Note: affected_archetypes no longer needed for conflict detection
        // Systems still query archetypes individually via their Query objects
    }

    const auto are_nodes_conflicting = [](const SystemNode& node, const SystemNode& other) {
        // Use const-aware access signature conflict detection
        return are_dependencies_overlapping(node.dependencies, other.dependencies);
    };

    for (std::size_t i = 0; i < num_nodes; ++i) {
        auto& node = (*system_nodes)[i];

        for (std::size_t j = i + 1; j < num_nodes; ++j) {
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
    for (std::size_t i = 0; i < num_nodes; ++i) {
        tasks[i] = systems_graph.emplace([this, i](tf::Subflow& subflow) {
            systems[i]->execute(get_engine(), subflow);
        });
    }

    for (std::size_t i = 0; i < num_nodes; ++i) {
        for (std::size_t j : system_deps[i]) {
            if (i < j) {
                tasks[i].precede(tasks[j]);
            }
        }
    }
}

auto Hephaestus::destroy_entity(Entity entity) -> void {
    destroy_queue.emplace_back(entity);
}
} // namespace atlas::hephaestus
