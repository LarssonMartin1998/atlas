#pragma once

#include <functional>
#include <typeindex>

#include "Concepts.hpp"

namespace atlas::core {
class IGame;
class IModule;
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

    [[nodiscard]] virtual auto get_game() -> IGame& = 0;

    template <TypeOfModule T>
    [[nodiscard]] auto get_module() const -> std::reference_wrapper<T>;

  protected:
    IEngine() = default;

    [[nodiscard]] virtual auto get_module_impl(std::type_index module) const
        -> IModule* = 0;
}; // namespace atlas::core

template <TypeOfModule T>
auto IEngine::get_module() const -> std::reference_wrapper<T> {
    IModule* module_interface = get_module_impl(std::type_index(typeid(T)));
    T* module_type = dynamic_cast<T*>(module_interface);
    return std::ref(*module_type);
}
} // namespace atlas::core
