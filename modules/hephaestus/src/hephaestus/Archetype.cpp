#include "hephaestus/Archetype.hpp"

namespace atlas::hephaestus {
auto Archetype::destroy_entity(Entity entity) -> bool {
    assert(ent_to_component_index.contains(entity) && "Entity does not exist in archetype");
    if (!ent_to_component_index.contains(entity)) {
        return false;
    }

    const auto last_component_index = component_storages.begin()->second->size() - 1;
    const auto entity_at_back = component_index_to_ent.at(last_component_index);

    const auto component_index_for_entity = ent_to_component_index.at(entity);
    for (auto& [component_type_id, storage_ptr] : component_storages) {
        (*storage_ptr).destroy(component_index_for_entity);
    }

    if (entity != entity_at_back) {
        ent_to_component_index[entity_at_back] = component_index_for_entity;
        component_index_to_ent[component_index_for_entity] = entity_at_back;
    }

    ent_to_component_index.erase(entity);
    component_index_to_ent.pop_back();
    return true;
}
} // namespace atlas::hephaestus
