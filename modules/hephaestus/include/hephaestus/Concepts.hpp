#pragma once

#include <type_traits>

namespace atlas::hephaestus {
class IComponent;
}

namespace atlas::hephaestus {
template <typename... Ts>
concept AllTypeOfComponent = (std::is_base_of_v<IComponent, Ts> && ...);

template <typename T>
concept TypeOfComponent = std::is_base_of_v<IComponent, T>;
} // namespace atlas::hephaestus
