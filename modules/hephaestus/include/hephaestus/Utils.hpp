#pragma once

#include <algorithm>
#include <typeindex>
#include <vector>

#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {

// New structure to represent a component access with const information
struct ComponentAccess {
    std::type_index type;
    bool is_const;
    
    auto operator==(const ComponentAccess& other) const -> bool {
        return type == other.type && is_const == other.is_const;
    }
    
    auto operator<(const ComponentAccess& other) const -> bool {
        if (type != other.type) {
            return type < other.type;
        }
        return is_const < other.is_const; // false < true, so non-const comes before const
    }
};

template <AllTypeOfComponent... ComponentTypes>
auto make_component_type_signature() -> std::vector<std::type_index> {
    auto type_indices = std::vector<std::type_index>{std::type_index(typeid(ComponentTypes))...};
    std::ranges::sort(type_indices);
    return type_indices;
}

// New function to create signatures with const information
template <typename... ComponentTypes>
auto make_component_access_signature() -> std::vector<ComponentAccess> {
    auto accesses = std::vector<ComponentAccess>{
        ComponentAccess{
            std::type_index(typeid(std::remove_cvref_t<ComponentTypes>)),
            std::is_const_v<std::remove_reference_t<ComponentTypes>>
        }...
    };
    std::sort(accesses.begin(), accesses.end());
    return accesses;
}

// New function to check access signature overlaps with const awareness
auto are_access_signatures_overlapping(
    const std::vector<ComponentAccess>& lhs,
    const std::vector<ComponentAccess>& rhs
) -> bool;

} // namespace atlas::hephaestus
