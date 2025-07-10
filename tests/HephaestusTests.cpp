#include <chrono>

#include <cstdint>
#include <gtest/gtest.h>

#include "atlas/core/Engine.hpp"
#include "atlas/core/IGame.hpp"
#include "hephaestus/ArchetypeKey.hpp"
#include "hephaestus/Component.hpp"
#include "hephaestus/Hephaestus.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephauestus::test {

using namespace atlas;
using namespace atlas::core;
using namespace atlas::hephaestus;

struct Position : public Component<Position> {
    float x, y;
};

struct Velocity : public Component<Velocity> {
    float dx, dy;
};

struct Health : public Component<Health> {
    int value;
};

struct TestState {
    int physics_runs = 0;
    int renderer_runs = 0;
    int health_checker_runs = 0;
    std::vector<Position> final_positions;
};

// clang-tidy rightfuly complains about this, however, it's fine for the test case.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static TestState* TEST_STATE = nullptr;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool USE_SHOULD_STOP = false;

class MockGame : public IGame {
  public:
    auto stop_game() -> void {
        should_stop = true;
    }

    auto pre_start() -> void override {}

    auto start() -> void override {
        auto& hephaestus = engine_ptr->get_module<Hephaestus>();

        hephaestus.create_entity(Position{.x = 1.F, .y = 2.F}, Velocity{.dx = 0.5F, .dy = -0.5F});
        hephaestus.create_entity(Position{.x = 3.F, .y = 4.F}, Velocity{.dx = 1.F, .dy = 1.F});
        hephaestus.create_entity(Position{.x = 5.F, .y = 6.F}, Health{.value = 100});

        if (TEST_STATE != nullptr) {
            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<Position&, const Velocity&> components) {
                auto& [pos, vel] = components;
                pos.x += vel.dx * 0.1F;
                pos.y += vel.dy * 0.1F;
                TEST_STATE->physics_runs++;
            });

            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<const Position&, const Velocity&> components) {
                const auto& [pos, vel] = components;
                TEST_STATE->renderer_runs++;
            });

            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<const Health&> components) {
                const auto& [health] = components;
                TEST_STATE->health_checker_runs++;
            });
        }
    }

    auto post_start() -> void override {
        start_time = std::chrono::steady_clock::now();
    }

    auto pre_shutdown() -> void override {}
    auto shutdown() -> void override {}
    auto post_shutdown() -> void override {}

    [[nodiscard]] auto get_engine() const -> IEngine& override {
        return *engine_ptr;
    }

    auto set_engine(IEngine& engine_ref) -> void override {
        engine_ptr = &engine_ref;
    }

    [[nodiscard]] auto should_quit() const -> bool override {
        if (USE_SHOULD_STOP) {
            return should_stop;
        }

        if (start_time.time_since_epoch().count() == 0) {
            return false;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        return elapsed.count() > 40;
    }

  private:
    IEngine* engine_ptr = nullptr;
    std::chrono::steady_clock::time_point start_time;
    bool should_stop = false;
};

TEST(HephaestusTest, SystemDependenciesGeneration) {
    auto const_signature = make_system_dependencies<const Position&, const Velocity&>();
    EXPECT_EQ(const_signature.size(), 2);
    EXPECT_TRUE(const_signature[0].is_read_only);
    EXPECT_TRUE(const_signature[1].is_read_only);

    // Test mixed const/non-const
    auto mixed_signature = make_system_dependencies<const Position&, Velocity&>();
    EXPECT_EQ(mixed_signature.size(), 2);

    // Find Position and Velocity in the sorted signature
    auto pos_it = std::ranges::find_if(mixed_signature, [](const SystemDependencies& access) {
        return access.type == std::type_index(typeid(Position));
    });
    auto vel_it = std::ranges::find_if(mixed_signature, [](const SystemDependencies& access) {
        return access.type == std::type_index(typeid(Velocity));
    });

    ASSERT_NE(pos_it, mixed_signature.end());
    ASSERT_NE(vel_it, mixed_signature.end());
    EXPECT_TRUE(pos_it->is_read_only);  // const Position&
    EXPECT_FALSE(vel_it->is_read_only); // Velocity&
}

TEST(HephaestusTest, SystemConflictDetection) {
    auto read_only_1 = make_system_dependencies<const Position&, const Velocity&>();
    auto read_only_2 = make_system_dependencies<const Position&>();
    EXPECT_FALSE(are_dependencies_overlapping(read_only_1, read_only_2));

    auto write_pos = make_system_dependencies<Position&, const Velocity&>();
    EXPECT_TRUE(are_dependencies_overlapping(read_only_1, write_pos));

    auto health_reader = make_system_dependencies<const Health&>();
    EXPECT_FALSE(are_dependencies_overlapping(read_only_1, health_reader));

    auto empty_access = make_system_dependencies<>();
    EXPECT_TRUE(empty_access.empty());

    auto pos_sig = make_system_dependencies<const Position&>();
    EXPECT_FALSE(are_dependencies_overlapping(empty_access, pos_sig));
    EXPECT_FALSE(are_dependencies_overlapping(pos_sig, empty_access));

    auto audio_sig = make_system_dependencies<const Position&>();
    auto ui_sig = make_system_dependencies<const Position&, const Velocity&>();
    auto logger_sig = make_system_dependencies<const Position&>();

    EXPECT_FALSE(are_dependencies_overlapping(audio_sig, ui_sig));
    EXPECT_FALSE(are_dependencies_overlapping(audio_sig, logger_sig));
    EXPECT_FALSE(are_dependencies_overlapping(ui_sig, logger_sig));

    auto physics_sig = make_system_dependencies<Position&, const Velocity&>();
    EXPECT_TRUE(are_dependencies_overlapping(audio_sig, physics_sig));
    EXPECT_TRUE(are_dependencies_overlapping(ui_sig, physics_sig));
    EXPECT_TRUE(are_dependencies_overlapping(logger_sig, physics_sig));
}

TEST(HephaestusTest, TypeSignatureGeneration) {
    auto signature = make_archetype_key<Position, Velocity>();
    EXPECT_EQ(signature.count_components(), 2);

    auto pos_id = get_component_type_id<Position>();
    auto vel_id = get_component_type_id<Velocity>();

    EXPECT_TRUE(signature.has_component(pos_id));
    EXPECT_TRUE(signature.has_component(vel_id));
}

TEST(HephaestusTest, HasDuplicateComponentTypeDetection) {
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Health>));

    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Position>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Position>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Health, Position>));

    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position&, const Position&>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Position&>));

    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position&, Position&>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<const Position&, Position&, Velocity>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<const Position, Position>));

    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Position, Health, Velocity>));

    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<int, float, double>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<int, float, int>));
}

TEST(HephaestusTest, ActualSystemCreationAndExecution) {
    USE_SHOULD_STOP = false;
    TestState state;
    TEST_STATE = &state;

    Engine<MockGame> engine;
    engine.run();

    EXPECT_GT(state.physics_runs, 0) << "Physics system should have run";
    EXPECT_GT(state.renderer_runs, 0) << "Renderer system should have run";
    EXPECT_GT(state.health_checker_runs, 0) << "Health checker should have run";

    TEST_STATE = nullptr;
}

TEST(HephaestusTest, ArchetypeKeyGeneration) {
    auto pos_sig = make_archetype_key<Position>();
    auto vel_sig = make_archetype_key<Velocity>();
    auto health_sig = make_archetype_key<Health>();
    auto empty_type = make_archetype_key<>();

    EXPECT_TRUE(empty_type.empty());
    EXPECT_NE(pos_sig, vel_sig);
    EXPECT_NE(vel_sig, health_sig);
    EXPECT_NE(pos_sig, health_sig);

    auto pos_vel_sig = make_archetype_key<Position, Velocity>();
    auto vel_health_sig = make_archetype_key<Velocity, Health>();

    EXPECT_NE(pos_vel_sig, pos_sig);
    EXPECT_NE(pos_vel_sig, vel_sig);
    EXPECT_NE(vel_health_sig, vel_sig);
    EXPECT_NE(vel_health_sig, health_sig);

    EXPECT_NE(pos_vel_sig, vel_health_sig);
}

TEST(HephaestusTest, ArchetypeKeyOperations) {
    auto pos_sig = make_archetype_key<Position>();
    auto vel_sig = make_archetype_key<Velocity>();
    auto pos_vel_sig = make_archetype_key<Position, Velocity>();

    EXPECT_TRUE(pos_sig.is_subset_of(pos_vel_sig));
    EXPECT_TRUE(vel_sig.is_subset_of(pos_vel_sig));
    EXPECT_FALSE(pos_vel_sig.is_subset_of(pos_sig));
    EXPECT_FALSE(pos_vel_sig.is_subset_of(vel_sig));

    EXPECT_TRUE(pos_sig.intersects_with(pos_vel_sig));
    EXPECT_TRUE(vel_sig.intersects_with(pos_vel_sig));
    EXPECT_FALSE(pos_sig.intersects_with(vel_sig));

    EXPECT_EQ(pos_sig.count_components(), 1);
    EXPECT_EQ(vel_sig.count_components(), 1);
    EXPECT_EQ(pos_vel_sig.count_components(), 2);
}

TEST(HephaestusTest, ArchetypeKeyConsistency) {
    auto sig1 = make_archetype_key<Position, Velocity>();
    auto sig2 = make_archetype_key<Velocity, Position>();

    EXPECT_EQ(sig1, sig2) << "Archetype key should be order-independent";

    auto sig3 = make_archetype_key<const Position, Velocity>();
    auto sig4 = make_archetype_key<Position, const Velocity>();
    auto sig5 = make_archetype_key<const Position, const Velocity>();

    EXPECT_EQ(sig1, sig3) << "const qualifiers should not affect archetype key";
    EXPECT_EQ(sig1, sig4) << "const qualifiers should not affect archetype key";
    EXPECT_EQ(sig1, sig5) << "const qualifiers should not affect archetype key";
}

TEST(HephaestusTest, ArchetypeKeyHash) {
    auto sig1 = make_archetype_key<Position, Velocity>();
    auto sig2 = make_archetype_key<Position, Velocity>();

    ArchetypeKeyHash hasher;
    EXPECT_EQ(hasher(sig1), hasher(sig2)) << "Equal keys should have equal hashes";

    auto sig3 = make_archetype_key<Position, Health>();
    EXPECT_NE(hasher(sig1), hasher(sig3)) << "Different keys should have different hashes";
}

TEST(HephaestusTest, ArchetypeMapCompatibility) {
    ArchetypeMap map;

    auto sig1 = make_archetype_key<Position>();
    auto sig2 = make_archetype_key<Position, Velocity>();

    map[sig1] = std::make_unique<Archetype>();
    map[sig2] = std::make_unique<Archetype>();

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(sig1));
    EXPECT_TRUE(map.contains(sig2));

    auto sig3 = make_archetype_key<Health>();
    EXPECT_FALSE(map.contains(sig3));
}

TEST(HephaestusTest, PerformanceComparison) {
    using namespace std::chrono;

    const auto num_iterations = 10000;

    auto start_test = high_resolution_clock::now();
    ArchetypeKeyHash hasher;
    for (int i = 0; i < num_iterations; ++i) {
        auto signature = make_archetype_key<Position, Velocity, Health>();
        volatile auto hash = hasher(signature); // volatile to prevent optimization
        (void)hash;                             // Avoid unused variable warning
    }
    auto end_test = high_resolution_clock::now();
    auto test_duration = duration_cast<microseconds>(end_test - start_test);

    std::cout << "ArchetypeKey system: " << test_duration.count() << " microseconds for "
              << num_iterations << " iterations\n";

    // Verify performance is reasonable (should complete in well under a second)
    EXPECT_LT(test_duration.count(), 1000000) << "ArchetypeKey operations should be fast";

    auto sig1 = make_archetype_key<Position, Velocity>();
    auto sig2 = make_archetype_key<Position, Health>();

    auto start_ops = high_resolution_clock::now();
    for (int i = 0; i < num_iterations; ++i) {
        volatile bool subset = sig1.is_subset_of(sig2);
        volatile bool intersects = sig1.intersects_with(sig2);
        volatile int count = sig1.count_components();
        (void)subset;
        (void)intersects;
        (void)count; // Avoid unused variable warnings
    }
    auto end_ops = high_resolution_clock::now();
    auto ops_duration = duration_cast<microseconds>(end_ops - start_ops);

    std::cout << "ArchetypeKey operations: " << ops_duration.count() << " microseconds for "
              << num_iterations << " iterations\n";

    // Operations should be very fast (constant time)
    EXPECT_LT(ops_duration.count(), 100000) // Less than 0.1 seconds
        << "ArchetypeKey operations should be constant time and very fast";
}

TEST(HephaestusTest, MemoryFootprintComparison) {
    auto signature = make_archetype_key<Position, Velocity, Health>();

    std::size_t signature_size = sizeof(signature);
    std::cout << "ArchetypeKey memory footprint: " << signature_size << " bytes\n";

    EXPECT_EQ(signature_size, sizeof(ArchetypeKey::StorageType))
        << "ArchetypeKey should only contain the storage array";

    EXPECT_EQ(signature.count_components(), 3);
    EXPECT_FALSE(signature.empty());
}

TEST(HephaestusTest, DeletingEntities) {
    class TestGameDelete : public MockGame {
      public:
        auto pre_start() -> void override {
            auto& hephaestus = get_engine().get_module<Hephaestus>();
            hephaestus.create_system(
                [this](const IEngine& engine, std::tuple<const Position&, const Health&> data) {
                    flip_flop = !flip_flop;
                }
            );
        }

        auto post_start() -> void override {
            auto& hephaestus = get_engine().get_module<Hephaestus>();
            EXPECT_EQ(hephaestus.get_tot_num_created_ents(), 0)
                << "Hephaestus should not have created any entities yet.";

            EXPECT_FALSE(flip_flop)
                << "Should not have run any system yet, flip_flop should be false.";

            hephaestus.tick();

            EXPECT_EQ(hephaestus.get_tot_num_created_ents(), 3)
                << "After running a single tick hephaestus shouldve created all three entities.";
            EXPECT_EQ(hephaestus.get_tot_num_destroyed_ents(), 0)
                << "We should not have destroyed any entities at this point yet.";
            EXPECT_TRUE(flip_flop) << "After running hephaestus.tick() flip_flop should be "
                                      "true after running a system update.";

            hephaestus.destroy_entity(2);
            EXPECT_EQ(
                hephaestus.get_tot_num_destroyed_ents(),
                0
            ) << "We have called destroy_entity, but since its batched into end of frame (and this "
                 "is outside of tick) it should not have destroyed anything yet, only queued it.";

            hephaestus.tick();
            EXPECT_EQ(hephaestus.get_tot_num_destroyed_ents(), 1)
                << "After running tick again we shouldve deleted the entity.";
            EXPECT_FALSE(flip_flop) << "Flip flop shouldve change value with this tick as well.";

            hephaestus.tick();
            EXPECT_FALSE(flip_flop) << "Flip flop should not change anymore. We destroyed "
                                       "the only entity for the system that updates it.";
            hephaestus.tick();
            EXPECT_FALSE(flip_flop) << "Flip flop should not change anymore. We destroyed "
                                       "the only entity for the system that updates it.";
            hephaestus.tick();
            EXPECT_FALSE(flip_flop) << "Flip flop should not change anymore. We destroyed "
                                       "the only entity for the system that updates it.";

            EXPECT_EQ(hephaestus.get_tot_num_destroyed_ents(), 1)
                << "Should have destroyed 1 entity";

            stop_game();
        }

      private:
        // Not thread safe but fine since we only have one entity for this.
        bool flip_flop = false;
    };

    USE_SHOULD_STOP = true;
    Engine<TestGameDelete> engine;
    engine.run();
}

TEST(HephaestusTest, PreCreateArchetypes) {
    struct CreateEntityComponent : Component<CreateEntityComponent> {
        std::uint8_t max_creations = 5;
    };

    struct UniqueComponent : Component<UniqueComponent> {};

    class TestArchetypeGame : public MockGame {
      public:
        auto pre_start() -> void override {
            auto& hephaestus = get_engine().get_module<Hephaestus>();

            hephaestus.create_entity(CreateEntityComponent{});
            hephaestus.create_system(
                [this,
                 &hephaestus](const IEngine& engine, std::tuple<CreateEntityComponent&> data) {
                    auto& [comp] = data;
                    if (creation_count < comp.max_creations) {
                        creation_count++;
                        // This would assert if we didn't pre-create the archetype.
                        hephaestus.create_entity(UniqueComponent{});
                    }
                }
            );

            // Without this, it would throw an assert later when we try to create an entity with
            // this archetype post start. We dont allow archetype/system creation post start for
            // thread scheduling and predictability reasons. However, entities must support creation
            // after start in a game. If you need create entities later than start, pre create the
            // archetype for that entity.
            hephaestus.create_archetype<UniqueComponent>();
        }

        auto post_start() -> void override {
            auto& hephaestus = get_engine().get_module<Hephaestus>();
            hephaestus.tick();
            hephaestus.tick();
            hephaestus.tick();
            hephaestus.tick();
            hephaestus.tick();

            EXPECT_EQ(creation_count, 5);

            stop_game();
        }

      private:
        std::uint8_t creation_count = 0;
    };

    USE_SHOULD_STOP = true;
    Engine<TestArchetypeGame>{}.run();
}
} // namespace atlas::hephauestus::test
