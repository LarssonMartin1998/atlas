#pragma once

#include "hephaestus/ArchetypeKey.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace atlas::hephaestus {
struct IComponentStorage {
    IComponentStorage() = default;

    IComponentStorage(const IComponentStorage&) = delete;
    IComponentStorage(IComponentStorage&&) = delete;

    auto operator=(const IComponentStorage&) -> IComponentStorage& = delete;
    auto operator=(IComponentStorage&&) -> IComponentStorage& = delete;

    virtual ~IComponentStorage() = default;

    [[nodiscard]] virtual auto size() const -> std::size_t = 0;
    virtual auto destroy(std::size_t index) -> void = 0;
};

template <TypeOfComponent ComponentType>
struct ComponentStorage final : public IComponentStorage {
    [[nodiscard]] auto size() const -> std::size_t override {
        return components.size();
    }

    auto destroy(std::size_t index) -> void override {
        assert(
            index < components.size()
            && "Invalid index when trying to destroy component in Archetype"
        );

        if (index != components.size() - 1) {
            components[index] = std::move(components.back());
        }

        components.pop_back();
    }

    std::vector<ComponentType> components{};
};

class Archetype final {
  public:
    explicit Archetype(const std::uint32_t entity_buffer_size) {
        ent_to_component_index.reserve(entity_buffer_size);
        component_index_to_ent.reserve(entity_buffer_size);
        component_storages.reserve(entity_buffer_size);
    }

    virtual ~Archetype() = default;

    Archetype(const Archetype&) = delete;
    auto operator=(const Archetype&) -> Archetype& = delete;

    Archetype(Archetype&&) = delete;
    auto operator=(Archetype&&) -> Archetype& = delete;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity(Entity entity, ComponentTypes&&... components) -> void;

    auto destroy_entity(Entity entity) -> bool;

    template <AllTypeOfComponent... ComponentTypes>
    auto get_entity_tuples() const -> decltype(auto);

  private:
    template <TypeOfComponent ComponentType>
    [[nodiscard]] auto get_components() const -> std::vector<ComponentType>&;

    template <TypeOfComponent ComponentType>
    auto add_to_component_storage(ComponentType&& component) -> void;

    using ComponentTypeId = std::size_t;
    std::unordered_map<Entity, std::size_t> ent_to_component_index;
    std::vector<Entity> component_index_to_ent;
    std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentStorage>> component_storages;
};

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::create_entity(Entity entity, ComponentTypes&&... components) -> void {
    (add_to_component_storage<ComponentTypes>(std::forward<ComponentTypes>(components)), ...);
    assert(!component_storages.empty() && "Cannot add entity without components");

    const auto& first_component_storage = *component_storages.begin()->second;
    ent_to_component_index.emplace(entity, first_component_storage.size() - 1);
    component_index_to_ent.emplace_back(entity);
}

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::get_entity_tuples() const -> decltype(auto) {
    return std::views::iota(std::size_t{0}, ent_to_component_index.size())
           | std::views::transform([this](const auto& index) {
                 return std::tuple<ComponentTypes&...>{get_components<ComponentTypes>()[index]...};
             });
}

template <TypeOfComponent ComponentType>
[[nodiscard]] auto Archetype::get_components() const -> std::vector<ComponentType>& {
    const auto type_id = get_component_type_id<ComponentType>();
    assert(component_storages.contains(type_id) && "Component type not found in archetype");

    return static_cast<ComponentStorage<ComponentType>*>(component_storages.at(type_id).get())
        ->components;
}

template <TypeOfComponent ComponentType>
auto Archetype::add_to_component_storage(ComponentType&& component) -> void {
    const auto type_id = get_component_type_id<ComponentType>();
    if (!component_storages.contains(type_id)) {
        component_storages.insert({type_id, std::make_unique<ComponentStorage<ComponentType>>()});
    }

    static_cast<ComponentStorage<ComponentType>&>(*component_storages[type_id].get())
        .components.emplace_back(std::forward<ComponentType>(component));
}
} // namespace atlas::hephaestus
