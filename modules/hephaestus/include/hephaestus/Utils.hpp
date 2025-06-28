#pragma once

#include <algorithm>
#include <typeindex>
#include <vector>

#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {

struct ComponentAccess {
    std::type_index type;
    bool is_read_only;

    auto operator==(const ComponentAccess& other) const -> bool {
        return type == other.type && is_read_only == other.is_read_only;
    }

    auto operator<=>(const ComponentAccess& other) const -> std::strong_ordering {
        if (type != other.type) {
            return type < other.type ? std::strong_ordering::less : std::strong_ordering::greater;
        }

        return static_cast<int>(is_read_only) <=> static_cast<int>(other.is_read_only);
    }
};

template <AllTypeOfComponent... ComponentTypes>
auto make_component_access_signature() -> std::vector<ComponentAccess> {
    auto accesses = std::vector<ComponentAccess>{ComponentAccess{
        std::type_index(typeid(std::remove_cvref_t<ComponentTypes>)),
        std::is_const_v<std::remove_reference_t<ComponentTypes>>
    }...};

    std::ranges::sort(accesses);
    return accesses;
}

auto are_access_signatures_overlapping(
    const std::vector<ComponentAccess>& lhs,
    const std::vector<ComponentAccess>& rhs
) -> bool;

// Extract basic type signature from access signature
inline auto extract_component_types(const std::vector<ComponentAccess>& access_signature) -> std::vector<std::type_index> {
    std::vector<std::type_index> types;
    types.reserve(access_signature.size());
    
    for (const auto& access : access_signature) {
        types.push_back(access.type);
    }
    
    // Remove duplicates while preserving sorted order
    types.erase(std::unique(types.begin(), types.end()), types.end());
    return types;
}

template <AllTypeOfComponent... ComponentTypes>
auto make_component_type_signature() -> std::vector<std::type_index> {
    auto type_indices = std::vector<std::type_index>{std::type_index(typeid(std::remove_cvref_t<ComponentTypes>))...};
    std::ranges::sort(type_indices);
    return type_indices;
}

} // namespace atlas::hephaestus
