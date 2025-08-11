#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/Component.hpp"

#include <cstddef>
#include <cstdint>
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

    constexpr auto ARCHETYPE_BUFFER_SIZE = 30;
    archetypes.reserve(ARCHETYPE_BUFFER_SIZE);
    ent_to_archetype_key.reserve(ARCHETYPE_BUFFER_SIZE);

    constexpr auto QUEUE_BUFFER_SIZE = 100;
    creation_queue.reserve(QUEUE_BUFFER_SIZE);
    destroy_queue.reserve(QUEUE_BUFFER_SIZE);
}

auto Hephaestus::start() -> void {}

auto Hephaestus::post_start() -> void {
    build_systems_dependency_graph();

    initialize_systems();
}

auto Hephaestus::shutdown() -> void {
    std::println("\nTotal created ents: {}", tot_num_created_ents);
    std::println("Total destroyed ents: {}", tot_num_destroyed_ents);
}

auto Hephaestus::tick() -> void {
    for (auto& creation : creation_queue) {
        creation();
    }
    tot_num_created_ents += creation_queue.size();
    creation_queue.clear();

    // TODO: Separate system caching into it's own frame step HERE and run it before executing the
    // systems. Additionally, create cache buckets for each archetype/version/tuple result instead
    // of storing it as one final result. This allows us to ONLY invalidate EXACTLY what is changed
    // in the cache, plus, with different buckets we can run the caching invalidation and rebuilding
    // in parallel. This should be a MAJOR increase in performance and predictability.

    if (!systems_graph.empty()) {
        systems_executor.run(systems_graph).wait();
    }

    // TODO: Handle and resolve all events here (non recursive, new events will be handled next
    // frame).

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
    tot_num_destroyed_ents += destroy_queue.size();
    destroy_queue.clear();
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

auto Hephaestus::initialize_systems() -> void {
    for (auto& system : systems) {
        (*system).cache_affected_archetypes(archetypes);
        (*system).create_query();
    }
}

auto Hephaestus::create_archetype_with_signature(
    const ArchetypeKey signature,
    const std::uint32_t entity_buffer_size
) -> void {
    const auto init_status = get_engine().get_engine_init_status();
    assert(
        init_status <= core::EngineInitStatus::RunningStart
        && "Cannot create new archetypes after start."
    );

    archetypes.emplace(signature, std::make_unique<Archetype>(entity_buffer_size));
}

auto Hephaestus::destroy_entity(Entity entity) -> void {
    destroy_queue.emplace_back(entity);
}

auto Hephaestus::get_tot_num_created_ents() const -> std::uint64_t {
    return tot_num_created_ents;
}

auto Hephaestus::get_tot_num_destroyed_ents() const -> std::uint64_t {
    return tot_num_destroyed_ents;
}
} // namespace atlas::hephaestus
