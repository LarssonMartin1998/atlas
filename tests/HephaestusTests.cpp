#include <chrono>

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

// Simple test components
struct Position : public Component<Position> {
    float x, y;
};
struct Velocity : public Component<Velocity> {
    float dx, dy;
};
struct Health : public Component<Health> {
    int value;
};

// Global test state
struct TestState {
    int physics_runs = 0;
    int renderer_runs = 0;
    int health_checker_runs = 0;
    std::vector<Position> final_positions;
};

// clang-tidy rightfuly complains about this, however, it's fine for the test case.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static TestState* TEST_STATE = nullptr;

// Mock game class for testing
class TestGame : public IGame {
  public:
    auto pre_start() -> void override {}
    auto start() -> void override {
        auto& hephaestus = engine_ptr->get_module<Hephaestus>().get();

        // Create some entities first
        hephaestus.create_entity(Position{.x = 1.F, .y = 2.F}, Velocity{.dx = 0.5F, .dy = -0.5F});
        hephaestus.create_entity(Position{.x = 3.F, .y = 4.F}, Velocity{.dx = 1.F, .dy = 1.F});
        hephaestus.create_entity(Position{.x = 5.F, .y = 6.F}, Health{.value = 100});

        if (TEST_STATE != nullptr) {
            // Create a physics system (writes to Position, reads Velocity)
            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<Position&, const Velocity&> components) {
                auto& [pos, vel] = components;
                pos.x += vel.dx * 0.1F; // Small delta for testing
                pos.y += vel.dy * 0.1F;
                TEST_STATE->physics_runs++;
            });

            // Create a renderer system (reads Position and Velocity)
            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<const Position&, const Velocity&> components) {
                const auto& [pos, vel] = components;
                // Simulate rendering
                TEST_STATE->renderer_runs++;
            });

            // Create a health checker system (reads Health)
            hephaestus.create_system([](const IEngine& engine,
                                        std::tuple<const Health&> components) {
                const auto& [health] = components;
                // Simulate health checking
                TEST_STATE->health_checker_runs++;
            });
        }
    }
    auto post_start() -> void override {
        // After everything is set up, prepare to quit after a few frames
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
        // Quit after a short time to let a few frames run
        if (start_time.time_since_epoch().count() == 0) {
            return false; // Not started yet
        }
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        return elapsed.count() > 100; // Quit after 100ms
    }

  private:
    IEngine* engine_ptr = nullptr;
    std::chrono::steady_clock::time_point start_time;
};

// Utility function tests (don't require full engine)
class HephaestusUtilsTest : public ::testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test 1: Basic const dependency generation
TEST_F(HephaestusUtilsTest, SystemDependenciesGeneration) {
    // Test basic const detection
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

// Test 2: Const-aware conflict detection
TEST_F(HephaestusUtilsTest, ConstAwareConflictDetection) {
    // Two read-only systems should NOT conflict
    auto read_only_1 = make_system_dependencies<const Position&, const Velocity&>();
    auto read_only_2 = make_system_dependencies<const Position&>();
    EXPECT_FALSE(are_dependencies_overlapping(read_only_1, read_only_2));

    // Read-only vs write should conflict
    auto write_pos = make_system_dependencies<Position&, const Velocity&>();
    EXPECT_TRUE(are_dependencies_overlapping(read_only_1, write_pos));

    // Non-overlapping components should not conflict
    auto health_reader = make_system_dependencies<const Health&>();
    EXPECT_FALSE(are_dependencies_overlapping(read_only_1, health_reader));
}

// Test 3: Type signature generation (for archetype queries)
TEST_F(HephaestusUtilsTest, TypeSignatureGeneration) {
    auto signature = make_archetype_key<Position, Velocity>();
    EXPECT_EQ(signature.count_components(), 2);

    // Test that the signature has the expected components
    auto pos_id = get_component_type_id<Position>();
    auto vel_id = get_component_type_id<Velocity>();

    EXPECT_TRUE(signature.has_component(pos_id));
    EXPECT_TRUE(signature.has_component(vel_id));
}

// Test 4: Edge cases
TEST_F(HephaestusUtilsTest, EdgeCases) {
    // Test empty access signature
    auto empty_access = make_system_dependencies<>();
    EXPECT_TRUE(empty_access.empty());

    // Test empty type signature
    auto empty_type = make_archetype_key<>();
    EXPECT_TRUE(empty_type.empty());

    // Test overlap with empty signatures
    auto pos_sig = make_system_dependencies<const Position&>();
    EXPECT_FALSE(are_dependencies_overlapping(empty_access, pos_sig));
    EXPECT_FALSE(are_dependencies_overlapping(pos_sig, empty_access));
}

TEST_F(HephaestusUtilsTest, ParallelExecutionBenefits) {
    // Create multiple read-only systems that should be able to run in parallel
    auto audio_sig = make_system_dependencies<const Position&>();
    auto ui_sig = make_system_dependencies<const Position&, const Velocity&>();
    auto logger_sig = make_system_dependencies<const Position&>();

    // The key benefit: these systems' signatures should not conflict
    EXPECT_FALSE(are_dependencies_overlapping(audio_sig, ui_sig));
    EXPECT_FALSE(are_dependencies_overlapping(audio_sig, logger_sig));
    EXPECT_FALSE(are_dependencies_overlapping(ui_sig, logger_sig));

    // But a write system should conflict with them
    auto physics_sig = make_system_dependencies<Position&, const Velocity&>();
    EXPECT_TRUE(are_dependencies_overlapping(audio_sig, physics_sig));
    EXPECT_TRUE(are_dependencies_overlapping(ui_sig, physics_sig));
    EXPECT_TRUE(are_dependencies_overlapping(logger_sig, physics_sig));
}

TEST_F(HephaestusUtilsTest, HasDuplicateComponentTypeDetection) {
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity>));
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Health>));

    // Test cases with duplicates
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Position>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Position>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Health, Position>));

    // Test with const variants (should still detect as duplicates)
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position&, const Position&>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Position&>));

    // Test with references and cv-qualifiers
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position&, Position&>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<const Position&, Position&, Velocity>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<const Position, Position>));

    // Multiple duplicates in one list
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<Position, Velocity, Position, Health, Velocity>));

    // Non-component types (to ensure it works with any types)
    EXPECT_FALSE((HAS_DUPLICATE_COMPONENT_TYPE_V<int, float, double>));
    EXPECT_TRUE((HAS_DUPLICATE_COMPONENT_TYPE_V<int, float, int>));
}

// Integration test that actually runs the engine
TEST(HephaestusIntegrationTest, ActualSystemCreationAndExecution) {
    // Set up test state
    TestState state;
    TEST_STATE = &state;

    // Create and run engine
    Engine<TestGame> engine;
    engine.run();

    // Verify systems actually executed
    // Each system should run once per frame for each matching entity
    // Physics and renderer systems match 2 entities (both have Position+Velocity)
    // Health checker matches 1 entity (only one has Health)
    // With 3 frames: physics=6, renderer=6, health=3
    EXPECT_GT(state.physics_runs, 0) << "Physics system should have run";
    EXPECT_GT(state.renderer_runs, 0) << "Renderer system should have run";
    EXPECT_GT(state.health_checker_runs, 0) << "Health checker should have run";

    // Clean up
    TEST_STATE = nullptr;
}

// Test class for the optimized signature system
class HephaestusOptimizedSignatureTest : public ::testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(HephaestusOptimizedSignatureTest, ArchetypeKeyGeneration) {
    // Test single component signature
    auto pos_sig = make_archetype_key<Position>();
    auto vel_sig = make_archetype_key<Velocity>();
    auto health_sig = make_archetype_key<Health>();

    // Signatures should be different
    EXPECT_NE(pos_sig, vel_sig);
    EXPECT_NE(vel_sig, health_sig);
    EXPECT_NE(pos_sig, health_sig);

    // Test multiple component signature
    auto pos_vel_sig = make_archetype_key<Position, Velocity>();
    auto vel_health_sig = make_archetype_key<Velocity, Health>();

    // Combined signatures should be different from individual ones
    EXPECT_NE(pos_vel_sig, pos_sig);
    EXPECT_NE(pos_vel_sig, vel_sig);
    EXPECT_NE(vel_health_sig, vel_sig);
    EXPECT_NE(vel_health_sig, health_sig);

    // Combined signatures should be different from each other
    EXPECT_NE(pos_vel_sig, vel_health_sig);
}

TEST_F(HephaestusOptimizedSignatureTest, ArchetypeKeyOperations) {
    auto pos_sig = make_archetype_key<Position>();
    auto vel_sig = make_archetype_key<Velocity>();
    auto pos_vel_sig = make_archetype_key<Position, Velocity>();

    // Test subset relationships
    EXPECT_TRUE(pos_sig.is_subset_of(pos_vel_sig));
    EXPECT_TRUE(vel_sig.is_subset_of(pos_vel_sig));
    EXPECT_FALSE(pos_vel_sig.is_subset_of(pos_sig));
    EXPECT_FALSE(pos_vel_sig.is_subset_of(vel_sig));

    // Test intersections
    EXPECT_TRUE(pos_sig.intersects_with(pos_vel_sig));
    EXPECT_TRUE(vel_sig.intersects_with(pos_vel_sig));
    EXPECT_FALSE(pos_sig.intersects_with(vel_sig));

    // Test component counting
    EXPECT_EQ(pos_sig.count_components(), 1);
    EXPECT_EQ(vel_sig.count_components(), 1);
    EXPECT_EQ(pos_vel_sig.count_components(), 2);
}

TEST_F(HephaestusOptimizedSignatureTest, ArchetypeKeyConsistency) {
    // Test that order doesn't matter for signature generation
    auto sig1 = make_archetype_key<Position, Velocity>();
    auto sig2 = make_archetype_key<Velocity, Position>();

    EXPECT_EQ(sig1, sig2) << "Archetype key should be order-independent";

    // Test that const/non-const doesn't affect signature
    auto sig3 = make_archetype_key<const Position, Velocity>();
    auto sig4 = make_archetype_key<Position, const Velocity>();
    auto sig5 = make_archetype_key<const Position, const Velocity>();

    EXPECT_EQ(sig1, sig3) << "const qualifiers should not affect archetype key";
    EXPECT_EQ(sig1, sig4) << "const qualifiers should not affect archetype key";
    EXPECT_EQ(sig1, sig5) << "const qualifiers should not affect archetype key";
}

TEST_F(HephaestusOptimizedSignatureTest, ArchetypeKeyHashPerformance) {
    // Test that signature hashing is consistent
    auto sig1 = make_archetype_key<Position, Velocity>();
    auto sig2 = make_archetype_key<Position, Velocity>();

    ArchetypeKeyHash hasher;
    EXPECT_EQ(hasher(sig1), hasher(sig2)) << "Equal keys should have equal hashes";

    // Test that different signatures have different hashes (basic collision test)
    auto sig3 = make_archetype_key<Position, Health>();
    EXPECT_NE(hasher(sig1), hasher(sig3)) << "Different keys should have different hashes";
}

TEST_F(HephaestusOptimizedSignatureTest, ArchetypeMapCompatibility) {
    // Test that ArchetypeMap can be created and used
    ArchetypeMap map;

    auto sig1 = make_archetype_key<Position>();
    auto sig2 = make_archetype_key<Position, Velocity>();

    // Test insertion and lookup
    map[sig1] = std::make_unique<Archetype>();
    map[sig2] = std::make_unique<Archetype>();

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(sig1));
    EXPECT_TRUE(map.contains(sig2));

    // Test that different signatures map to different entries
    auto sig3 = make_archetype_key<Health>();
    EXPECT_FALSE(map.contains(sig3));
}

TEST_F(HephaestusOptimizedSignatureTest, PerformanceComparison) {
    using namespace std::chrono;

    const auto num_iterations = 10000;

    // Test signature creation and hashing performance
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
    EXPECT_LT(test_duration.count(), 1000000) // Less than 1 second
        << "ArchetypeKey operations should be fast";

    // Test signature operations performance
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

TEST_F(HephaestusOptimizedSignatureTest, MemoryFootprintComparison) {
    // Test memory footprint of the new ArchetypeKey system
    auto signature = make_archetype_key<Position, Velocity, Health>();

    // The new signature system uses a fixed-size array of uint64_t values
    size_t signature_size = sizeof(signature);

    // For comparison, estimate what a vector-based signature would cost
    // (3 type_index elements + vector overhead)
    size_t estimated_vector_size = sizeof(std::vector<std::type_index>)
                                   + (3 * sizeof(std::type_index));

    std::cout << "ArchetypeKey memory footprint: " << signature_size << " bytes\n";
    std::cout << "Estimated vector-based signature: " << estimated_vector_size << " bytes\n";

    // Verify that we have a reasonable memory footprint
    EXPECT_EQ(signature_size, sizeof(ArchetypeKey::StorageType))
        << "ArchetypeKey should only contain the storage array";

    // Verify signature functionality
    EXPECT_EQ(signature.count_components(), 3);
    EXPECT_FALSE(signature.empty());
}

// Demonstration of complete drop-in replacement
TEST_F(HephaestusOptimizedSignatureTest, DropInReplacementDemo) {
    // This test demonstrates how the optimized system could be used as a complete drop-in
    // replacement In a real integration, you would include "hephaestus/OptimizedECS.hpp" instead of
    // the legacy headers

    // Create optimized archetype map
    ArchetypeMap optimized_archetypes;

    // Use the optimized signature generation
    auto key = make_archetype_key<Position, Velocity>();

    // Add archetype to the map
    optimized_archetypes[key] = std::make_unique<Archetype>();

    // Test lookup performance
    EXPECT_TRUE(optimized_archetypes.contains(key));
    EXPECT_EQ(optimized_archetypes.size(), 1);

    // Add more entities with different signatures
    auto health_key = make_archetype_key<Position, Health>();
    auto full_key = make_archetype_key<Position, Velocity, Health>();

    optimized_archetypes[health_key] = std::make_unique<Archetype>();
    optimized_archetypes[full_key] = std::make_unique<Archetype>();

    EXPECT_EQ(optimized_archetypes.size(), 3);
    EXPECT_TRUE(optimized_archetypes.contains(health_key));
    EXPECT_TRUE(optimized_archetypes.contains(full_key));

    // Demonstrate signature operations
    EXPECT_TRUE(key.is_subset_of(full_key));
    EXPECT_TRUE(health_key.is_subset_of(full_key));
    EXPECT_TRUE(key.intersects_with(full_key));
    EXPECT_TRUE(key.intersects_with(health_key)); // Both have Position component

    // Test signatures that don't intersect
    auto velocity_only = make_archetype_key<Velocity>();
    auto health_only = make_archetype_key<Health>();
    EXPECT_FALSE(velocity_only.intersects_with(health_only)); // No shared components

    std::cout << "Drop-in replacement demo: Successfully managed " << optimized_archetypes.size()
              << " archetypes with optimized signatures\n";
}
} // namespace atlas::hephauestus::test
