#pragma once

#include "hephaestus/Archetype.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include <functional>

namespace atlas {
namespace core {
class IEngine;
}
namespace hephaestus {
class ArchetypeBase;
} // namespace hephaestus
} // namespace atlas

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class System final : public SystemBase {
  public:
    using SystemFunc = std::function<void(
        const core::IEngine &, std::vector<std::tuple<ComponentTypes &...>> &)>;
    explicit System(SystemFunc func) : func{func} {}

    auto update_archetypes(
        std::vector<std::reference_wrapper<ArchetypeBase>> &&matches) -> void {
        this->matches = std::move(matches);
    }

    auto execute(const core::IEngine &engine) -> void override {
        std::vector<std::tuple<ComponentTypes &...>> components;

        for (const auto &match : matches) {
            auto &archetype =
                static_cast<Archetype<ComponentTypes...> &>(match.get());
            const auto entity_components = archetype.get_components();
            // It's not guaranteed that all of the components from the
            // archetype are included in the system. All we KNOW, is that
            // the archetype contains AT LEAST the components specified for
            // this system. We need to transform the components from the
            // archetype into the components that the system expects.
            for (auto &tuple : entity_components) {
                components.emplace_back(std::get<ComponentTypes>(tuple)...);
            }
        }

        func(engine, components);
    }

  private:
    std::vector<std::reference_wrapper<ArchetypeBase>> matches;
    SystemFunc func;
};

} // namespace atlas::hephaestus
