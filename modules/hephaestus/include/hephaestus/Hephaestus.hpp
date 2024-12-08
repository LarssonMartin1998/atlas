#pragma once

#include <algorithm>
#include <functional>
#include <print>
#include <ranges>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/ITickable.hpp"
#include "core/Module.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeBase.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/Hashing.hpp"
#include "hephaestus/System.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
class Hephaestus final : public core::Module, public core::ITickable {
  public:
    using ArchetypePtr = std::unique_ptr<ArchetypeBase>;
    using ArchetypeMap =
        std::unordered_map<std::vector<std::type_index>, ArchetypePtr,
                           TypeIndexVectorHash, TypeIndexVectorEqual>;

    explicit Hephaestus(core::IEngine &engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;

    template <AllTypeOfComponent... ComponentTypes, typename Func>
    auto create_system(Func &&func) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity() -> void;

  protected:
    [[nodiscard]] static auto generate_unique_entity_id() -> Entity;

    // Retrieve a filtered view of archetypes that hold at least the components
    // specified in the template parameter list.
    template <AllTypeOfComponent... ComponentTypes>
    [[nodiscard]] auto get_matching_archetypes()
        -> std::vector<std::reference_wrapper<ArchetypeBase>>;

  private:
    std::vector<std::unique_ptr<SystemBase>> systems;
    ArchetypeMap archetypes;
};

template <AllTypeOfComponent... ComponentTypes, typename Func>
auto Hephaestus::create_system(Func &&func) -> void {
    auto new_system =
        std::make_unique<System<ComponentTypes...>>(std::forward<Func>(func));
    new_system->update_archetypes(get_matching_archetypes<ComponentTypes...>());
    systems.emplace_back(std::move(new_system));
}

template <AllTypeOfComponent... ComponentTypes>
auto Hephaestus::create_entity() -> void {
    const auto entity_id = generate_unique_entity_id();
    const auto signature = make_component_type_signature<ComponentTypes...>();

    auto &archetype = [this, signature]() -> ArchetypePtr & {
        if (!archetypes.contains(signature)) {
            archetypes.emplace(
                signature, std::make_unique<Archetype<ComponentTypes...>>());
        }

        return archetypes[signature];
    }();

    static_cast<Archetype<ComponentTypes...> *>(archetype.get())
        ->create_entity(entity_id);
}

template <AllTypeOfComponent... ComponentTypes>
[[nodiscard]] auto Hephaestus::get_matching_archetypes()
    -> std::vector<std::reference_wrapper<ArchetypeBase>> {
    const auto signature = make_component_type_signature<ComponentTypes...>();
    auto signature_set =
        std::unordered_set<std::type_index>{signature.begin(), signature.end()};

    auto matches_signature = [&signature_set](const auto &pair) {
        const auto &key = pair.first;
        return std::ranges::all_of(
            signature_set, [&key](const auto &component) {
                return std::ranges::find(key, component) != key.end();
            });
    };

    std::vector<std::reference_wrapper<ArchetypeBase>> matches;

    // Iterate over the filtered and transformed view and collect references
    for (auto &archetype_ref :
         archetypes | std::ranges::views::filter(matches_signature) |
             std::ranges::views::transform([](auto &pair) -> ArchetypeBase & {
                 return *(pair.second);
             })) {
        matches.emplace_back(archetype_ref);
    }

    return matches;
}
} // namespace atlas::hephaestus
