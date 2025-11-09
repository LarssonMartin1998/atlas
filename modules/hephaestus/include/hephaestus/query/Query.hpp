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
    mutable std::vector<ComponentsCache<ComponentTypes...>> cache_buckets;
    std::vector<std::reference_wrapper<Archetype>> filtered_archetypes;
};

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] inline auto Query<ComponentTypes...>::get() const
    -> std::vector<ComponentsCache<ComponentTypes...>>& {
    for (std::size_t i = 0; i < cache_buckets.size(); i++) {
        const auto& archetype = filtered_archetypes[i];
        const auto current_version = archetype.get().get_version();
        if (current_version != cache_buckets[i].last_version) {
            const auto& entity_tuples = archetype.get().get_entity_tuples<ComponentTypes...>();
            // We evaluate the pipeline and collect it into a vector.
            // This costs one iteration over the data, but enables size storage and
            // random access. This can be used to chunk and parellize the execution
            // of the systems. And should result in better performance and
            // utilization.
            cache_buckets[i].components = std::ranges::to<std::vector>(entity_tuples);
            cache_buckets[i].last_version = current_version;
        }
    }

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
} // namespace atlas::hephaestus
