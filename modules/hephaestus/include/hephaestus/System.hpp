#pragma once

#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/query/Query.hpp"
#include <functional>
#include <print>
#include <utility>

namespace atlas::core {
class IEngine;
}

namespace atlas::hephaestus {
template <AllTypeOfComponent... ComponentTypes>
class System final : public SystemBase {
  public:
    using SystemFunc = std::function<void(const core::IEngine&,
                                          const Query<ComponentTypes...>&)>;

    explicit System(SystemFunc func, const ArchetypeMap& archetypes,
                    std::vector<std::type_index>&& component_types)
        : func{std::move(func)}, query{archetypes, std::move(component_types)} {
    }

    System(const System&) = delete;
    auto operator=(const System&) -> System& = delete;

    System(System&&) = delete;
    auto operator=(System&&) -> System& = delete;

    ~System() override = default;

    auto execute(const core::IEngine& engine) -> void override {
        func(engine, query);
    }

  private:
    Query<ComponentTypes...> query;
    SystemFunc func;
};

} // namespace atlas::hephaestus
