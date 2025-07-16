#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <print>
#include <unordered_map>

#include "core/Concepts.hpp"
#include "core/IEngine.hpp"
#include "core/IModule.hpp"
#include "core/ITickable.hpp"
#include "core/ModulesFactory.hpp"
#include "core/time/EngineClock.hpp"

namespace atlas::core {
template <TypeOfGame G>
class Engine final : public IEngine {
  public:
    Engine() = default;
    ~Engine() override;

    Engine(const Engine&) = delete;
    auto operator=(const Engine&) -> Engine& = delete;

    Engine(Engine&&) = delete;
    auto operator=(Engine&&) -> Engine& = delete;

    auto run() -> void override;

    [[nodiscard]] auto get_game() -> IGame& override;

    [[nodiscard]] auto get_clock() const -> const IEngineClock& override;
    [[nodiscard]] auto get_engine_init_status() const -> EngineInitStatus override;

  protected:
    [[nodiscard]] auto get_module_impl(std::type_index module) const -> IModule* override;

  private:
    auto tick_root() -> void;

    G game;

    std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules;
    std::vector<ITickable*> ticking_modules;

    EngineClock clock;

    EngineInitStatus init_status = EngineInitStatus::NotInitialized;
};

template <TypeOfGame G>
Engine<G>::~Engine() {
    game.shutdown();

    for (auto& [module_type, module] : modules) {
        module->shutdown();
    }
}

template <TypeOfGame G>
auto Engine<G>::run() -> void {
    game.set_engine(*this);

    init_status = EngineInitStatus::RunningPreStart;
    create_modules(*this, modules, ticking_modules);
    for (auto& [module_type, module] : modules) {
        module->start();
    }
    game.pre_start();

    init_status = EngineInitStatus::RunningStart;
    for (auto& [module_type, module] : modules) {
        module->pre_start();
    }
    game.start();

    init_status = EngineInitStatus::RunningPostStart;
    for (auto& [module_type, module] : modules) {
        module->post_start();
    }
    game.post_start();

    init_status = EngineInitStatus::Initialized;

    std::uint64_t num_frames = 0;
    while (!game.should_quit()) {
        if (num_frames == 1) {
            clock.start_post_first_frame_timer();
        }

        tick_root();

        clock.update_frame_timers(num_frames);
        num_frames++;

        const auto& dt = clock.get_delta_time();
        constexpr auto FRAME_SPIKE_THRESHOLD = 0.0015;
        if (dt > FRAME_SPIKE_THRESHOLD) {
            std::println("⚠️ Frame spike: {} ms", dt * 1000);
        }
    }

    const auto total_time = clock.get_total_time();
    std::println("--------------------");
    std::println("\nNum frames: {}", num_frames);
    std::println("Total runtime: {}\n", total_time);

    if (const auto result = clock.get_total_time_without_first_frame(); result.has_value()) {
        const auto total_tick_time = *result;
        std::println("First frame: {}\n", total_time - total_tick_time);
        std::println(
            "avg FPS(excluding first frame): {}",
            static_cast<double>(num_frames) / total_tick_time
        );
    }
    std::println("avg FPS: {}", static_cast<double>(num_frames) / total_time);

    std::println("\navg frame time: {} ms", clock.get_avg_frame_time() * 1000);
    std::println("fastest frame: {} ms", clock.get_fastest_frame_time() * 1000);
    std::println("slowest frame: {} ms", clock.get_slowest_frame_time() * 1000);
}

template <TypeOfGame G>
auto Engine<G>::get_game() -> IGame& {
    return game;
}

template <TypeOfGame G>
auto Engine<G>::get_engine_init_status() const -> EngineInitStatus {
    return init_status;
}

template <TypeOfGame G>
auto Engine<G>::get_module_impl(std::type_index module) const -> IModule* {
    assert(modules.contains(module));
    const auto& module_ptr = modules.at(module);
    return module_ptr.get();
}

template <TypeOfGame G>
auto Engine<G>::get_clock() const -> const IEngineClock& {
    return clock;
}

template <TypeOfGame G>
auto Engine<G>::tick_root() -> void {
    for (auto* module : ticking_modules) {
        module->tick();
    }
}
} // namespace atlas::core
