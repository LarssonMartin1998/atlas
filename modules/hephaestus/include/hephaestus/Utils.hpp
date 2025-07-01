#pragma once

#include <algorithm>
#include <typeindex>
#include <vector>

#include "hephaestus/ComponentSignature.hpp"
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

template <typename...>
struct HasDuplicateComponentType : std::false_type {};

template <typename T, typename... Rest>
struct HasDuplicateComponentType<T, Rest...>
    : std::bool_constant<
          (std::is_same_v<std::decay_t<T>, std::decay_t<Rest>> || ...)
          || HasDuplicateComponentType<Rest...>::value> {};

template <typename... Ts>
constexpr bool HAS_DUPLICATE_COMPONENT_TYPE_V = HasDuplicateComponentType<Ts...>::value;

template <AllTypeOfComponent... ComponentTypes>
auto make_component_access_signature() -> std::vector<ComponentAccess> {
    auto accesses = std::vector<ComponentAccess>{ComponentAccess{
        .type = std::type_index(typeid(std::remove_cvref_t<ComponentTypes>)),
        .is_read_only = std::is_const_v<std::remove_reference_t<ComponentTypes>>
    }...};

    std::ranges::sort(accesses, [](const ComponentAccess& lhs, const ComponentAccess& rhs) {
        return lhs.type < rhs.type;
    });

    return accesses;
}

auto are_access_signatures_overlapping(
    const std::vector<ComponentAccess>& lhs,
    const std::vector<ComponentAccess>& rhs
) -> bool;

template <AllTypeOfComponent... ComponentTypes>
auto make_component_type_signature() -> ComponentSignature {
    ComponentSignature signature;
    ((signature.add_component(get_component_type_id<ComponentTypes>())), ...);
    return signature;
}
} // namespace atlas::hephaestus
