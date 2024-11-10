#pragma once

#include "ModuleTraits.hpp"
#include <optional>

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

    template <typename T>
    [[nodiscard]] auto
    get_module() const -> std::optional<std::reference_wrapper<T>>;

  protected:
    IEngine() = default;

    [[nodiscard]] virtual auto
    get_module_impl(EModules module) const -> std::optional<IModule *> = 0;
}; // namespace atlas::core

template <typename T>
auto IEngine::get_module() const -> std::optional<std::reference_wrapper<T>> {
    static_assert(std::is_base_of<IModule, T>::value,
                  "T must derive from IModule");

    std::optional<IModule *> module =
        get_module_impl(ModuleTraits<T>::module_enum);
    if (!module.has_value()) {
        return std::nullopt;
    }

    return std::ref(*module.value());
}
} // namespace atlas::core
