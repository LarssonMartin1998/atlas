#pragma once

#include <type_traits>

namespace atlas::core {
class IModule;

template <typename T>
concept TypeOfModule = std::is_base_of_v<IModule, T>;
} // namespace atlas::core
