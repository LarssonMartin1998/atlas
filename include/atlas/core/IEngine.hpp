#pragma once

#include <functional>

#include "Concepts.hpp"
#include "core/ModuleTraits.hpp"

namespace atlas::core {
class IGame;
class IModule;
enum class EModules;
} // namespace atlas::core

namespace atlas::core {
class IEngine {
  public:
    virtual ~IEngine() = default;

    IEngine(const IEngine&) = delete;
    auto operator=(const IEngine&) -> IEngine& = delete;

    IEngine(IEngine&&) = delete;
    auto operator=(IEngine&&) -> IEngine& = delete;

    virtual auto run() -> void = 0;

    [[nodiscard]] virtual auto get_game() const
        -> std::reference_wrapper<IGame> = 0;

    template <TypeOfModule T>
    [[nodiscard]] auto get_module() const -> std::reference_wrapper<T>;

  protected:
    IEngine() = default;

    [[nodiscard]] virtual auto get_module_impl(EModules module) const
        -> IModule* = 0;
}; // namespace atlas::core

template <TypeOfModule T>
auto IEngine::get_module() const -> std::reference_wrapper<T> {
    IModule* module_interface = get_module_impl(ModuleTraits<T>::module_enum);
    T* module_type = dynamic_cast<T*>(module_interface);
    return std::ref(*module_type);
}
} // namespace atlas::core
