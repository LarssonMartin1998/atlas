#include "hephaestus/Hephaestus.hpp"
#include "core/IEngine.hpp"
#include "core/threads/IThreadPool.hpp"

#include <print>

namespace atlas::hephaestus {
Hephaestus::Hephaestus(core::IEngine& engine) : core::Module{engine} {
    constexpr auto queue_buffert = 100;
    creation_queue.reserve(queue_buffert);
    destroy_queue.reserve(queue_buffert);
    std::println("Hephaestus Constructor");
}

auto Hephaestus::start() -> void { std::println("Hephaestus::start()"); }
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
} // namespace atlas::hephaestus
