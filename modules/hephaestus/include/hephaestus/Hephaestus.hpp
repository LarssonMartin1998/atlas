#pragma once

#include <typeindex>
#include <unordered_map>
#include <vector>

#include "core/Module.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeBase.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/Hashing.hpp"
#include "hephaestus/IHephaestus.hpp"

namespace atlas::hephaestus {
class Hephaestus final : public IHephaestus, public core::Module {
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

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity() -> void;

  protected:
    [[nodiscard]] auto generate_unique_entity_id() -> Entity override;

  private:
    ArchetypeMap archetypes;
};

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
} // namespace atlas::hephaestus
