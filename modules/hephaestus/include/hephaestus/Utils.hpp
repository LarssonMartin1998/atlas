#pragma once

#include <algorithm>
#include <ranges>
#include <typeindex>
#include <vector>

#include "hephaestus/Concepts.hpp"
#include "hephaestus/ComponentSignature.hpp"

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
auto make_component_type_signature() -> std::vector<std::type_index> {
    auto type_indices = std::vector<std::type_index>{
        std::type_index(typeid(std::remove_cvref_t<ComponentTypes>))...
    };
    std::ranges::sort(type_indices);
    return type_indices;
}

// Optimized bitmask-based signature generation
template <AllTypeOfComponent... ComponentTypes>
constexpr auto make_component_signature() -> ComponentSignature {
    ComponentSignature signature;
    ((signature.add_component(get_component_bitmask<ComponentTypes>())), ...);
    return signature;
}

// Conversion function from ComponentAccess vector to ComponentSignature
inline auto convert_access_signature_to_component_signature(
    const std::vector<ComponentAccess>& access_signature
) -> ComponentSignature {
    ComponentSignature signature;
    for (const auto& access : access_signature) {
        // This is a placeholder implementation
        // In a real implementation, we'd need a registry to map std::type_index to component IDs
        // For now, we'll use a simple hash-based approach
        const auto hash = access.type.hash_code();
        const auto bit_pos = hash % 64;
        signature.add_component(1ULL << bit_pos);
    }
    return signature;
}
} // namespace atlas::hephaestus
