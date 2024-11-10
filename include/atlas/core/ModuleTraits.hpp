#pragma once

#include "EModules.hpp"

namespace atlas::core {
class ECSModule;

template <typename T> struct ModuleTraits;

template <> struct ModuleTraits<ECSModule> {
    static constexpr EModules module_enum = EModules::ECS;
};
} // namespace atlas::core
