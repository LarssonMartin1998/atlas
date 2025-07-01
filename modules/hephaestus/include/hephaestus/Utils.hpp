#pragma once

#include <algorithm>
#include <ranges>
#include <typeindex>
#include <unordered_map>
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

// Runtime registry for type_index to component ID mapping
// This is populated automatically when component signatures are created
inline auto get_type_index_to_component_id_map() -> std::unordered_map<std::type_index, size_t>& {
    static std::unordered_map<std::type_index, size_t> map;
    return map;
}

// Register a component type in the runtime mapping
template<typename T>
auto register_component_type() -> void {
    const auto component_id = get_component_type_id<T>();
    get_type_index_to_component_id_map()[std::type_index(typeid(T))] = component_id;
}

// Convert ComponentAccess vector to ComponentSignature
inline auto component_access_to_signature(const std::vector<ComponentAccess>& access_vector) -> ComponentSignature {
    ComponentSignature signature;
    const auto& map = get_type_index_to_component_id_map();
    
    for (const auto& access : access_vector) {
        auto it = map.find(access.type);
        if (it != map.end()) {
            signature.add_component(it->second);
        }
    }
    return signature;
}

template <AllTypeOfComponent... ComponentTypes>
auto make_component_type_signature() -> ComponentSignature {
    // Register component types in runtime mapping
    (register_component_type<ComponentTypes>(), ...);
    
    ComponentSignature signature;
    ((signature.add_component(get_component_type_id<ComponentTypes>())), ...);
    return signature;
}
} // namespace atlas::hephaestus
