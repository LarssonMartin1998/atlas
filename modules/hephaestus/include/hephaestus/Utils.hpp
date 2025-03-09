#pragma once

#include <algorithm>
#include <typeindex>
#include <vector>

#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
auto make_component_type_signature() -> std::vector<std::type_index> {
    auto type_indices = std::vector<std::type_index>{
        std::type_index(typeid(ComponentTypes))...};
    std::ranges::sort(type_indices);
    return type_indices;
}
} // namespace atlas::hephaestus
