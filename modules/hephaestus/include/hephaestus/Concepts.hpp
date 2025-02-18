#pragma once

#include <type_traits>

namespace atlas::hephaestus {
class IComponent;
}

namespace atlas::hephaestus {
template <typename T>
concept NotConst = !std::is_const_v<T>;

template <typename... Ts>
concept AllTypeOfComponent =
    (NotConst<Ts> && ...) && (std::is_base_of_v<IComponent, Ts> && ...);

template <typename T>
concept TypeOfComponent = NotConst<T> && std::is_base_of_v<IComponent, T>;
} // namespace atlas::hephaestus
