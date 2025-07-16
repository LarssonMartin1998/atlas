#pragma once

#include <algorithm>
#include <typeindex>
#include <vector>

#include "hephaestus/ArchetypeKey.hpp"
#include "hephaestus/Concepts.hpp"

namespace atlas::hephaestus {

struct SystemDependencies {
    std::type_index type;
    bool is_read_only;

    constexpr auto operator==(const SystemDependencies& other) const -> bool {
        return type == other.type && is_read_only == other.is_read_only;
    }

    constexpr auto operator<=>(const SystemDependencies& other) const -> std::strong_ordering {
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
auto make_system_dependencies() -> std::vector<SystemDependencies> {
    auto accesses = std::vector<SystemDependencies>{SystemDependencies{
        .type = std::type_index(typeid(std::remove_cvref_t<ComponentTypes>)),
        .is_read_only = std::is_const_v<std::remove_reference_t<ComponentTypes>>
    }...};

    std::ranges::sort(accesses, [](const SystemDependencies& lhs, const SystemDependencies& rhs) {
        return lhs.type < rhs.type;
    });

    return accesses;
}

auto are_dependencies_overlapping(
    const std::vector<SystemDependencies>& lhs,
    const std::vector<SystemDependencies>& rhs
) -> bool;

template <AllTypeOfComponent... ComponentTypes>
auto make_archetype_key() -> ArchetypeKey {
    ArchetypeKey signature;
    ((signature.add_component(get_component_type_id<ComponentTypes>())), ...);
    return signature;
}
} // namespace atlas::hephaestus
