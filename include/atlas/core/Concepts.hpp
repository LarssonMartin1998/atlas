#pragma once

#include <type_traits>

namespace atlas::core {
class IModule;
class IGame;

template <typename T>
concept TypeOfModule = std::is_base_of_v<IModule, T>;

template <typename T>
concept TypeOfGame = std::is_base_of_v<IGame, T>;
} // namespace atlas::core
