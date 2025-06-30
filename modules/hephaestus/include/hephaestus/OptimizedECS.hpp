#pragma once

// This header provides drop-in replacements for the ECS signature optimization
// When included, it will replace the legacy signature system with the optimized one

#include "hephaestus/ComponentSignature.hpp"
#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {

// Drop-in replacement for make_component_type_signature() using optimized system
template <AllTypeOfComponent... ComponentTypes>
constexpr auto make_component_type_signature() -> ComponentSignature {
    return make_component_signature<ComponentTypes...>();
}

// Drop-in replacement for ArchetypeMap using optimized system
using ArchetypeMap = OptimizedArchetypeMap;

} // namespace atlas::hephaestus