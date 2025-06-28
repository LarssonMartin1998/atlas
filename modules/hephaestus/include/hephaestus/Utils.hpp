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

} // namespace atlas::hephaestus
