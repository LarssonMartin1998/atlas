#pragma once

#include <functional>

#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/query/QueryComponentsPipeline.hpp"

namespace atlas::hephaestus {

template <AllTypeOfComponent... ComponentTypes>
using ComponentsVector = std::vector<std::tuple<ComponentTypes&...>>;

template <AllTypeOfComponent... ComponentTypes>
struct ComponentsCache {
  public:
    ComponentsVector<ComponentTypes...> components;

  private:
    template <AllTypeOfComponent... FriendComponentTypes>
    friend class Query;

    ArchetypeVersion last_version;
};

template <AllTypeOfComponent... ComponentTypes>
class Query final {
  public:
    Query() = default;

    Query(const Query&) = delete;
    auto operator=(const Query&) = delete;

    Query(const Query&&) = delete;
    auto operator=(const Query&&) = delete;

    ~Query() = default;

  private:
  public:
    [[nodiscard]]
    inline auto get() const -> std::vector<ComponentsCache<ComponentTypes...>>&;

    inline auto set_archetypes(const ArchetypeMap& archetypes) -> void;

  private:
    inline auto perform_cache_maintenance() const;
    inline auto partial_add_to_cache(
        const Archetype& archetype,
        ComponentsCache<ComponentTypes...>& cache_bucket,
        const cache_recording::RecordedChange& change
    ) const;
    inline auto partial_remove_from_cache(
        ComponentsCache<ComponentTypes...>& cache_bucket,
        const cache_recording::RecordedChange& change
    ) const;

    mutable std::vector<ComponentsCache<ComponentTypes...>> cache_buckets;
    std::vector<std::reference_wrapper<Archetype>> filtered_archetypes;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get() const
    -> std::vector<ComponentsCache<ComponentTypes...>>& {
    perform_cache_maintenance();
    return cache_buckets;
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::set_archetypes(const ArchetypeMap& archetypes) -> void {
    filtered_archetypes = filter_archetypes<ComponentTypes...>(archetypes)
                          | std::ranges::views::transform(
                              [&](const auto& pair) -> std::reference_wrapper<Archetype> {
                                  auto& archetype = *pair.second;
                                  return std::ref(archetype);
                              }
                          )
                          | std::ranges::to<std::vector>();

    cache_buckets = std::vector<ComponentsCache<ComponentTypes...>>();
    cache_buckets.resize(filtered_archetypes.size());
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::perform_cache_maintenance() const {
    for (std::size_t i = 0; i < cache_buckets.size(); i++) {
        auto& cache_bucket = cache_buckets[i];

        const auto& archetype = filtered_archetypes[i].get();
        const auto current_version = archetype.get_version();
        if (current_version == cache_bucket.last_version) {
            continue;
        }

        // If our cache changes is half or more of the total entities we just rebuild the entire
        // thing. Dont expect any performance gains from patching up that large of a cache miss.
        const auto num_entities = archetype.get_num_entities();
        const auto changes = archetype.get_recorded_changes();
        const auto should_rebuild_cache = changes.size() * 2 >= num_entities;
        if (should_rebuild_cache) {
            const auto& entity_tuples = archetype.get_entity_tuples<ComponentTypes...>();
            // We evaluate the pipeline and collect it into a vector.
            // This costs one iteration over the data, but enables size storage and
            // random access. This can be used to chunk and parellalize the execution
            // of the systems. And should result in better performance and
            // utilization. Plus allows for easily updating the cache partially on the fly.
            cache_bucket.components = std::ranges::to<std::vector>(entity_tuples);
            cache_bucket.last_version = current_version;
        } else {
            for (const auto& change : changes) {
                using namespace cache_recording;
                switch (change.action) {
                case RecordedChangeAction::AddEntity:
                    partial_add_to_cache(archetype, cache_bucket, change);
                    break;
                case RecordedChangeAction::DestroyEntity:
                    partial_remove_from_cache(cache_bucket, change);
                    break;
                }
            }
        }
    }
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::partial_add_to_cache(
    const Archetype& archetype,
    ComponentsCache<ComponentTypes...>& cache_bucket,
    const cache_recording::RecordedChange& change
) const {
    cache_bucket.components.emplace_back(
        archetype.get_entity_tuple<ComponentTypes...>(change.component_index)
    );
}

template <AllTypeOfComponent... ComponentTypes>
inline auto Query<ComponentTypes...>::partial_remove_from_cache(
    ComponentsCache<ComponentTypes...>& cache_bucket,
    const cache_recording::RecordedChange& change
) const {
    auto& components = cache_bucket.components;
    const auto last_index = components.size() - 1;

    if (last_index != change.component_index) {
        components[change.component_index] = std::move(components[last_index]);
    }

    components.pop_back();
}

} // namespace atlas::hephaestus
