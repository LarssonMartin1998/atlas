#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/ArchetypeQueryContext.hpp"
#include "hephaestus/Concepts.hpp"
#include <algorithm>
#include <print>
#include <ranges>

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes> class Query final {
  public:
    inline explicit Query(const ArchetypeMap& archetypes,
                          std::vector<std::type_index>&& component_types);

    Query(const Query&) = delete;
    auto operator=(const Query&) -> Query& = delete;

    Query(Query&&) = delete;
    auto operator=(Query&&) -> Query& = delete;

    ~Query() = default;

    // Retrieve a filtered view of archetypes that hold at least the components
    // specified in the template parameter list.
    [[nodiscard]] inline auto get() const -> decltype(auto);

  private:
    const ArchetypeQueryContext context;
};

template <AllTypeOfComponent... ComponentTypes>
inline Query<ComponentTypes...>::Query(
    const ArchetypeMap& archetypes,
    std::vector<std::type_index>&& component_types)
    : context{archetypes, {component_types.begin(), component_types.end()}} {
    std::println("Query Constructor");
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::get() const -> decltype(auto) {
    return context.archetypes |
           std::ranges::views::filter([this](const auto& pair) {
               const auto& archetype_component_types = pair.first;

               return std::ranges::all_of(
                   context.signature,
                   [&archetype_component_types](
                       const auto& signature_component_type) {
                       return std::ranges::find(archetype_component_types,
                                                signature_component_type) !=
                              archetype_component_types.end();
                   });
           }) |
           std::ranges::views::transform([this](const auto& pair) {
               const auto& archetype = *pair.second;
               return archetype.template get_entity_tuples<ComponentTypes...>();
           }) |
           std::ranges::views::join;
}
} // namespace atlas::hephaestus
