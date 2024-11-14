#pragma once

#include "EModules.hpp"

namespace atlas {
namespace core {
template <typename T> struct ModuleTraits;
}

#define MODULE_TRAIT(ModuleClassName, module_namespace_name)                   \
    namespace module_namespace_name {                                          \
    class ModuleClassName;                                                     \
    }                                                                          \
    namespace core {                                                           \
    template <> struct ModuleTraits<module_namespace_name::ModuleClassName> {  \
        static constexpr EModules module_enum = EModules::ModuleClassName;     \
    };                                                                         \
    }

// Use the macro to define a forward declaration and a ModuleTraits
// specialization
MODULE_TRAIT(Hephaestus, hephaestus)

#undef MODULE_TRAIT
} // namespace atlas
