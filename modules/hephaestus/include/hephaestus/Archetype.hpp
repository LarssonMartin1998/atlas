#pragma once

#include "hephaestus/ArchetypeKey.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include <cassert>
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
};

template <TypeOfComponent ComponentType>
struct ComponentStorage final : public IComponentStorage {
    [[nodiscard]] auto size() const -> std::size_t override {
        return components.size();
    }

    std::vector<ComponentType> components{};
};

class Archetype final {
  public:
    Archetype() {
        constexpr auto ENTITY_BUFFERT = 500;
        entities.reserve(ENTITY_BUFFERT);
        component_storages.reserve(ENTITY_BUFFERT);
    }

    virtual ~Archetype() = default;

    Archetype(const Archetype&) = delete;
    auto operator=(const Archetype&) -> Archetype& = delete;

    Archetype(Archetype&&) = delete;
    auto operator=(Archetype&&) -> Archetype& = delete;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity(Entity entity, ComponentTypes&&... components) -> void;

    auto destroy_entity(Entity entity) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto get_entity_tuples() const -> decltype(auto);

  private:
    template <TypeOfComponent ComponentType>
    [[nodiscard]] auto get_components() const -> std::vector<ComponentType>&;

    template <TypeOfComponent ComponentType>
    auto add_to_component_storage(ComponentType&& component) -> void;

    std::unordered_map<Entity, std::size_t> entities;
    std::unordered_map<std::size_t, std::unique_ptr<IComponentStorage>> component_storages;
};

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::create_entity(Entity entity, ComponentTypes&&... components) -> void {
    (add_to_component_storage<ComponentTypes>(std::forward<ComponentTypes>(components)), ...);
    assert(!component_storages.empty() && "Cannot add entity without components");

    const auto& component_storage = *component_storages.begin()->second;
    entities.emplace(entity, component_storage.size());
}

auto Archetype::destroy_entity(Entity entity) -> void {
    assert(entities.contains(entity) && "Entity does not exist in archetype");
    // TODO
}

template <AllTypeOfComponent... ComponentTypes>
auto Archetype::get_entity_tuples() const -> decltype(auto) {
    return std::views::iota(std::size_t{0}, entities.size())
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

    ComponentType::version_counter++;
}
} // namespace atlas::hephaestus
