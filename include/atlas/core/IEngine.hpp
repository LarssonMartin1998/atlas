#pragma once

#include "ModuleTraits.hpp"
#include <functional>

namespace atlas::core {
class IGame;
class IModule;
enum class EModules;

class IEngine {
  public:
    virtual ~IEngine() = default;

    IEngine(const IEngine &) = delete;
    auto operator=(const IEngine &) -> IEngine & = delete;

    IEngine(IEngine &&) = delete;
    auto operator=(IEngine &&) -> IEngine & = delete;

    virtual auto run() -> void = 0;

    [[nodiscard]] virtual auto
    get_game() const -> std::reference_wrapper<IGame> = 0;

    template <typename T>
    [[nodiscard]] auto get_module() const -> std::reference_wrapper<T>;

  protected:
    IEngine() = default;

    [[nodiscard]] virtual auto
    get_module_impl(EModules module) const -> IModule * = 0;
}; // namespace atlas::core

template <typename T>
auto IEngine::get_module() const -> std::reference_wrapper<T> {
    static_assert(std::is_base_of<IModule, T>::value,
                  "T must derive from IModule");

    IModule *module = get_module_impl(ModuleTraits<T>::module_enum);
    return std::ref(*module);
}
} // namespace atlas::core
