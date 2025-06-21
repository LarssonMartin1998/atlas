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

auto compare_signatures(const std::vector<std::type_index>& lhs,
                        const std::vector<std::type_index>& rhs) -> bool;

auto are_signatures_overlapping(const std::vector<std::type_index>& lhs,
                                const std::vector<std::type_index>& rhs)
    -> bool;
} // namespace atlas::hephaestus
