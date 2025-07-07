#pragma once

#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

#include <taskflow/taskflow.hpp>

#include "core/IEngine.hpp"
#include "core/ITickable.hpp"
#include "core/Module.hpp"
#include "hephaestus/Archetype.hpp"
#include "hephaestus/ArchetypeKey.hpp"
#include "hephaestus/ArchetypeMap.hpp"
#include "hephaestus/Common.hpp"
#include "hephaestus/Concepts.hpp"
#include "hephaestus/System.hpp"
#include "hephaestus/SystemBase.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
template <typename T>
struct Debug;

template <typename... Ts>
struct Debugs;

struct SystemNode {
    std::vector<SystemDependencies> dependencies;
};

class Hephaestus final : public core::Module, public core::ITickable {
  public:
    explicit Hephaestus(core::IEngine& engine);

    auto start() -> void override;
    auto post_start() -> void override;

    auto shutdown() -> void override;

    auto tick() -> void override;

    template <typename Func>
    auto create_system(Func&& func) -> void;

    template <AllTypeOfComponent... ComponentTypes>
    auto create_entity(ComponentTypes&&... components) -> void;

    auto destroy_entity(Entity entity) -> void;

  protected:
    [[nodiscard]] static auto generate_unique_entity_id() -> Entity;

    auto build_systems_dependency_graph() -> void;

  private:
    std::vector<std::unique_ptr<SystemBase>> systems;
    ArchetypeMap archetypes;
    std::unordered_map<Entity, ArchetypeKey> ent_to_archetype_key;

    std::vector<std::function<void()>> creation_queue;
    std::vector<Entity> destroy_queue;
    std::optional<std::vector<SystemNode>> system_nodes = std::vector<SystemNode>{};

    tf::Taskflow systems_graph;
    tf::Executor systems_executor;

    std::size_t tot_num_created_ents = 0;
    std::size_t tot_num_destroyed_ents = 0;

    // This is all confusing, however, the purpose of this is to improve the API
    // for calling the create_system function. This way, the user only needs to
    // pass the lambda which will be used as the system function, the rest is
    // deduced and handled.

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
    template <typename ClassType, typename ReturnType, typename EngineParam, typename TupleParam>
    struct FunctionTraits<ReturnType (ClassType::*)(EngineParam, TupleParam) const> {
        static_assert(
            !std::is_const_v<std::remove_reference_t<TupleParam>>,
            "Const tuples are not supported. Use std::tuple<const Component&, ...>& instead of "
            "const std::tuple<Component&, ...>&"
        );
        using EngineType = std::decay_t<EngineParam>;
        using TupleType = std::remove_reference_t<TupleParam>; // Remove reference but keep
                                                               // component const-ness
    };

    template <typename T>
    struct TupleElements;

    template <typename... Ts>
    struct TupleElements<std::tuple<Ts...>> {
        static_assert(
            !HAS_DUPLICATE_COMPONENT_TYPE_V<Ts...>,
            "A system cannot take the same component type twice (const or non-const)."
        );

        template <template <typename...> class Template>
        using Apply = Template<std::remove_cvref_t<Ts>...>; // Remove both const and ref

        static auto make_dependencies() {
            return make_system_dependencies<Ts...>();
        }
    };
};

template <typename Func>
auto Hephaestus::create_system(Func&& func) -> void {
    const auto init_status = get_engine().get_engine_init_status();
    assert(
        init_status == core::EngineInitStatus::RunningStart
        && "Cannot create systems after startup."
    );
    assert(
        system_nodes != std::nullopt
        && "Trying to create a system after the system_nodes have been reset."
    );

    using Traits = FunctionTraits<std::decay_t<Func>>;
    using TupleType = typename Traits::TupleType; // e.g. std::tuple<Transform&, Velocity&>
    using Components = TupleElements<TupleType>;
    using SystemType = typename Components::template Apply<System>;

    auto dependencies = Components::make_dependencies();
    system_nodes->emplace_back(SystemNode{.dependencies = dependencies});

    auto new_system = std::make_unique<SystemType>(
        std::forward<Func>(func),
        archetypes,
        std::move(dependencies)
    );

    systems.emplace_back(std::move(new_system));
}

// No entities are created on the fly. We enqueue all of it into a collection
// which is iterated and constructs all entities in the begining of the next
// frame.
template <AllTypeOfComponent... ComponentTypes>
auto Hephaestus::create_entity(ComponentTypes&&... components) -> void {
    static_assert(
        !HAS_DUPLICATE_COMPONENT_TYPE_V<ComponentTypes...>,
        "A single entity cannot have the same component type twice (const or non-const)."
    );

    auto components_tuple = std::make_tuple(std::forward<ComponentTypes>(components)...);

    creation_queue.emplace_back([this, data = std::move(components_tuple)]() mutable {
        const auto entity_id = generate_unique_entity_id();
        const auto signature = make_archetype_key<ComponentTypes...>();

        auto& archetype = [this, &signature]() -> ArchetypePtr& {
            if (!archetypes.contains(signature)) {
                archetypes.emplace(signature, std::make_unique<Archetype>());
            }

            return archetypes[signature];
        }();

        std::apply(
            [&](auto&&... unpacked) {
                archetype->template create_entity<ComponentTypes...>(
                    entity_id,
                    std::move(unpacked)...
                );
            },
            data
        );

        ent_to_archetype_key.emplace(entity_id, signature);
    });
}
} // namespace atlas::hephaestus
