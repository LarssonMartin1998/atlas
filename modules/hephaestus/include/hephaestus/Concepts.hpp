#pragma once

#include <type_traits>

namespace atlas::hephaestus {
template <typename Derived>
class Component;
}

namespace atlas::hephaestus {

template <typename T>
concept TypeOfComponent = std::
    is_base_of_v<Component<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>;

template <typename... Ts>
concept AllTypeOfComponent = (TypeOfComponent<Ts> && ...);

template <typename T>
concept RValueArg = !std::is_lvalue_reference_v<T>;
} // namespace atlas::hephaestus
