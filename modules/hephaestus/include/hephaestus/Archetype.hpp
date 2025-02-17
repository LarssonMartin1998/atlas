#pragma once

#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include <cassert>
#include <memory>
#include <print>
#include <ranges>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace atlas::hephaestus {
struct IComponentStorage {};

template <TypeOfComponent ComponentType>
struct ComponentStorage final : public IComponentStorage {
    std::vector<ComponentType> components{};
};

class Archetype final {
  public:
    Archetype() = default;
    virtual ~Archetype() = default;

    Archetype(const Archetype&) = delete;
    auto operator=(const Archetype&) -> Archetype& = delete;

    Archetype(Archetype&&) = delete;
    auto operator=(Archetype&&) -> Archetype& = delete;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity(Entity entity) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto get_entity_tuples() const -> decltype(auto);

  private:
    template <TypeOfComponent ComponentType>
    [[nodiscard]] auto get_components() const -> std::vector<ComponentType>&;

    template <TypeOfComponent ComponentType>
    auto add_to_component_storage() -> void;

    std::vector<Entity> entities;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>>
        component_storages;
};

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::create_entity(Entity entity) -> void {
    entities.emplace_back(entity);
    (add_to_component_storage<ComponentTypes>(), ...);
}

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::get_entity_tuples() const -> decltype(auto) {
    return std::views::iota(std::size_t{0}, entities.size()) |
           std::views::transform([this](const auto& index) {
               return std::tuple<ComponentTypes&...>{
                   get_components<ComponentTypes>()[index]...};
           });
}

template <TypeOfComponent ComponentType>
[[nodiscard]] auto Archetype::get_components() const
    -> std::vector<ComponentType>& {
    const auto component_type = std::type_index(typeid(ComponentType));
    assert(component_storages.contains(component_type) &&
           "Component type not found in archetype");

    return static_cast<ComponentStorage<ComponentType>*>(
               component_storages.at(component_type).get())
        ->components;
}

template <TypeOfComponent ComponentType>
auto Archetype::add_to_component_storage() -> void {
    const auto type_index = std::type_index(typeid(ComponentType));
    if (!component_storages.contains(type_index)) {
        component_storages.insert(
            {type_index, std::make_unique<ComponentStorage<ComponentType>>()});
    }

    static_cast<ComponentStorage<ComponentType>&>(
        *component_storages[type_index].get())
        .components.emplace_back(ComponentType{});
}
} // namespace atlas::hephaestus
