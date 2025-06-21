#pragma once

#include <cassert>
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
template <TypeOfGame G> class Engine final : public IEngine {
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
    [[nodiscard]] auto get_engine_init_status() const
        -> EngineInitStatus override;

  protected:
    [[nodiscard]] auto get_module_impl(std::type_index module) const
        -> IModule* override;

  private:
    auto tick_root() -> void;

    G game;

    std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules;
    std::vector<ITickable*> ticking_modules;

    EngineClock clock;

    EngineInitStatus init_status = EngineInitStatus::NotInitialized;
};

template <TypeOfGame G> Engine<G>::~Engine() {
    game.shutdown();

    for (auto& [module_type, module] : modules) {
        module->shutdown();
    }

    std::println("Engine destroyed");
}

template <TypeOfGame G> auto Engine<G>::run() -> void {
    std::println("Engine::run()");

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

    auto num_frames = 0;
    while (!game.should_quit()) {
        tick_root();

        clock.update_delta_time();
        num_frames++;
    }

    std::println("Num frames: {}", num_frames);

    // const auto path = std::string("perf_stats/no_changes");
    // std::filesystem::create_directories(path);
    //
    // const auto file_count = std::count_if(
    //     std::filesystem::directory_iterator(path),
    //     std::filesystem::directory_iterator(),
    //     [](auto const& entry) { return entry.is_regular_file(); });
    //
    // const auto file_name = path + "/" + std::to_string(file_count) + ".txt";
    // {
    //     std::ofstream ofs(file_name);
    //     if (!ofs) {
    //         std::println("Failed to open file: {}", file_name);
    //     } else {
    //         ofs << num_frames << "\n";
    //         std::println("Wrote {} frames to {}", num_frames, file_name);
    //     }
    // }
}

template <TypeOfGame G> auto Engine<G>::get_game() -> IGame& { return game; }

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

template <TypeOfGame G> auto Engine<G>::tick_root() -> void {
    for (auto* module : ticking_modules) {
        module->tick();
    }
}
} // namespace atlas::core
