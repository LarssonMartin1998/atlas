#pragma once

#include <algorithm>
#include <tuple>
#include <vector>

#include "core/ITickable.hpp"
#include "core/Module.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/System.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
class Hephaestus;
}

namespace atlas::hephaestus {
class Hephaestus final : public core::Module, public core::ITickable {
  public:
    explicit Hephaestus(core::IEngine &engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;

    template <typename Func> auto create_system(Func &&func) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity() -> void;

  protected:
    [[nodiscard]] static auto generate_unique_entity_id() -> Entity;

  private:
    std::vector<std::unique_ptr<SystemBase>> systems;
    ArchetypeMap archetypes;

    // This is all confusing, however, the purpose of this is to improve the API
    // for calling the create_system function. This way, the user only needs to
    // pass the lambda which will be used as the system function, the rest is
    // deduced and handled.
    //
    // TODO: This can later be used to improve the API for creating entities,
    // letting the user only pass the actual created components with or without
    // values, and let this deduce it and create archetypes from it.

    // A utility to pull out parameter types from a a callable.
    template <typename T>
    struct FunctionTraits : FunctionTraits<decltype(&T::operator())> {};
    // This leverages the call operator of a lambda:
    // For example, if we have:
    //   auto myLambda = [](int a, float b) { ... };
    // then decltype(&decltype(myLambda)::operator()) = Ret (ClassType::*)(int,
    // float) const;

    // Now the partial specialization for a non-generic, const lambda
    // with exactly two parameters.
    template <typename ClassType, typename ReturnType, typename EngineParam,
              typename QueryParam>

    struct FunctionTraits<ReturnType (ClassType:: *)(EngineParam, QueryParam)
                              const> {
        using EngineType = std::decay_t<EngineParam>;
        using QueryType = std::decay_t<QueryParam>;
    };

    // extracts the pack from `Query<Components...>`
    template <typename T> struct QueryTraits; // primary template, no definition

    // partial specialization
    template <AllTypeOfComponent... Components>
    struct QueryTraits<Query<Components...>> {
        using ComponentTuple = std::tuple<Components...>;
    };
};

template <typename Func> auto Hephaestus::create_system(Func &&func) -> void {
    using Traits = FunctionTraits<std::decay_t<Func>>;
    using QueryType =
        typename Traits::QueryType; // e.g. Query<Transform, Velocity>
    using QueryTraits = QueryTraits<QueryType>;
    using CompTuple =
        typename QueryTraits::ComponentTuple; // std::tuple<Transform,

    // "expand" that tuple to get ComponentTypes...
    // So we can do: System<ComponentTypes...>
    std::apply(
        [&](auto... dummy) {
            // dummy are placeholders of type components types
            // But they are all value-initialized (like Transform(),
            // Velocity()). Only need the *types*:
            using SystemType = System<std::decay_t<decltype(dummy)>...>;

            auto new_system = std::make_unique<SystemType>(
                std::forward<Func>(func), archetypes,
                make_component_type_signature<
                    std::decay_t<decltype(dummy)>...>());

            systems.emplace_back(std::move(new_system));
        },
        CompTuple{});
}

template <AllTypeOfComponent... ComponentTypes>
auto Hephaestus::create_entity() -> void {
    const auto entity_id = generate_unique_entity_id();
    const auto signature = make_component_type_signature<ComponentTypes...>();

    auto &archetype = [this, signature]() -> ArchetypePtr & {
        if (!archetypes.contains(signature)) {
            archetypes.emplace(signature, std::make_unique<Archetype>());
        }

        return archetypes[signature];
    }();

    archetype.get()->template create_entity<ComponentTypes...>(entity_id);
}
} // namespace atlas::hephaestus
