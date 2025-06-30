#include <atlas/core/Engine.hpp>
#include <atlas/core/IGame.hpp>
#include <chrono>
#include <gtest/gtest.h>
#include <hephaestus/Component.hpp>
#include <hephaestus/Hephaestus.hpp>
#include <hephaestus/Utils.hpp>

using namespace atlas;
using namespace atlas::hephaestus;

// Simple test components
struct Position : public atlas::hephaestus::Component<Position> {
    float x, y;
};
struct Velocity : public atlas::hephaestus::Component<Velocity> {
    float dx, dy;
};
struct Health : public atlas::hephaestus::Component<Health> {
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
class TestGame : public atlas::core::IGame {
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
            hephaestus.create_system([](const atlas::core::IEngine& engine,
                                        std::tuple<Position&, const Velocity&> components) {
                auto& [pos, vel] = components;
                pos.x += vel.dx * 0.1F; // Small delta for testing
                pos.y += vel.dy * 0.1F;
                TEST_STATE->physics_runs++;
            });

            // Create a renderer system (reads Position and Velocity)
            hephaestus.create_system([](const atlas::core::IEngine& engine,
                                        std::tuple<const Position&, const Velocity&> components) {
                const auto& [pos, vel] = components;
                // Simulate rendering
                TEST_STATE->renderer_runs++;
            });

            // Create a health checker system (reads Health)
            hephaestus.create_system([](const atlas::core::IEngine& engine,
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
    [[nodiscard]] auto get_engine() const -> atlas::core::IEngine& override {
        return *engine_ptr;
    }
    auto set_engine(atlas::core::IEngine& engine_ref) -> void override {
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
    atlas::core::IEngine* engine_ptr = nullptr;
    std::chrono::steady_clock::time_point start_time;
};

// Utility function tests (don't require full engine)
class HephaestusUtilsTest : public ::testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test 1: Basic const signature generation
TEST_F(HephaestusUtilsTest, ComponentAccessSignatureGeneration) {
    // Test basic const detection
    auto const_signature = make_component_access_signature<const Position&, const Velocity&>();
    EXPECT_EQ(const_signature.size(), 2);
    EXPECT_TRUE(const_signature[0].is_read_only);
    EXPECT_TRUE(const_signature[1].is_read_only);

    // Test mixed const/non-const
    auto mixed_signature = make_component_access_signature<const Position&, Velocity&>();
    EXPECT_EQ(mixed_signature.size(), 2);

    // Find Position and Velocity in the sorted signature
    auto pos_it = std::ranges::find_if(mixed_signature, [](const ComponentAccess& access) {
        return access.type == std::type_index(typeid(Position));
    });
    auto vel_it = std::ranges::find_if(mixed_signature, [](const ComponentAccess& access) {
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
    auto read_only_1 = make_component_access_signature<const Position&, const Velocity&>();
    auto read_only_2 = make_component_access_signature<const Position&>();
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_1, read_only_2));

    // Read-only vs write should conflict
    auto write_pos = make_component_access_signature<Position&, const Velocity&>();
    EXPECT_TRUE(are_access_signatures_overlapping(read_only_1, write_pos));

    // Non-overlapping components should not conflict
    auto health_reader = make_component_access_signature<const Health&>();
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_1, health_reader));
}

// Test 3: Type signature generation (for archetype queries)
TEST_F(HephaestusUtilsTest, TypeSignatureGeneration) {
    auto signature = make_component_type_signature<Position, Velocity>();
    EXPECT_EQ(signature.size(), 2);

    // Should contain type indices for Position and Velocity
    auto has_position = std::ranges::find(signature, std::type_index(typeid(Position)))
                        != signature.end();
    auto has_velocity = std::ranges::find(signature, std::type_index(typeid(Velocity)))
                        != signature.end();

    EXPECT_TRUE(has_position);
    EXPECT_TRUE(has_velocity);
}

// Test 4: Edge cases
TEST_F(HephaestusUtilsTest, EdgeCases) {
    // Test empty access signature
    auto empty_access = make_component_access_signature<>();
    EXPECT_TRUE(empty_access.empty());

    // Test empty type signature
    auto empty_type = make_component_type_signature<>();
    EXPECT_TRUE(empty_type.empty());

    // Test overlap with empty signatures
    auto pos_sig = make_component_access_signature<const Position&>();
    EXPECT_FALSE(are_access_signatures_overlapping(empty_access, pos_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(pos_sig, empty_access));
}

TEST_F(HephaestusUtilsTest, ParallelExecutionBenefits) {
    // Create multiple read-only systems that should be able to run in parallel
    auto audio_sig = make_component_access_signature<const Position&>();
    auto ui_sig = make_component_access_signature<const Position&, const Velocity&>();
    auto logger_sig = make_component_access_signature<const Position&>();

    // The key benefit: these systems' signatures should not conflict
    EXPECT_FALSE(are_access_signatures_overlapping(audio_sig, ui_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(audio_sig, logger_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(ui_sig, logger_sig));

    // But a write system should conflict with them
    auto physics_sig = make_component_access_signature<Position&, const Velocity&>();
    EXPECT_TRUE(are_access_signatures_overlapping(audio_sig, physics_sig));
    EXPECT_TRUE(are_access_signatures_overlapping(ui_sig, physics_sig));
    EXPECT_TRUE(are_access_signatures_overlapping(logger_sig, physics_sig));
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
    atlas::core::Engine<TestGame> engine;
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

TEST_F(HephaestusOptimizedSignatureTest, ComponentSignatureGeneration) {
    using namespace atlas::hephaestus;
    
    // Test single component signature
    auto pos_sig = make_component_signature<Position>();
    auto vel_sig = make_component_signature<Velocity>();
    auto health_sig = make_component_signature<Health>();
    
    // Signatures should be different
    EXPECT_NE(pos_sig, vel_sig);
    EXPECT_NE(vel_sig, health_sig);
    EXPECT_NE(pos_sig, health_sig);
    
    // Test multiple component signature
    auto pos_vel_sig = make_component_signature<Position, Velocity>();
    auto vel_health_sig = make_component_signature<Velocity, Health>();
    
    // Combined signatures should be different from individual ones
    EXPECT_NE(pos_vel_sig, pos_sig);
    EXPECT_NE(pos_vel_sig, vel_sig);
    EXPECT_NE(vel_health_sig, vel_sig);
    EXPECT_NE(vel_health_sig, health_sig);
    
    // Combined signatures should be different from each other
    EXPECT_NE(pos_vel_sig, vel_health_sig);
}

TEST_F(HephaestusOptimizedSignatureTest, ComponentSignatureOperations) {
    using namespace atlas::hephaestus;
    
    auto pos_sig = make_component_signature<Position>();
    auto vel_sig = make_component_signature<Velocity>();
    auto pos_vel_sig = make_component_signature<Position, Velocity>();
    
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

TEST_F(HephaestusOptimizedSignatureTest, ComponentSignatureConsistency) {
    using namespace atlas::hephaestus;
    
    // Test that order doesn't matter for signature generation
    auto sig1 = make_component_signature<Position, Velocity>();
    auto sig2 = make_component_signature<Velocity, Position>();
    
    EXPECT_EQ(sig1, sig2) << "Component signature should be order-independent";
    
    // Test that const/non-const doesn't affect signature
    auto sig3 = make_component_signature<const Position, Velocity>();
    auto sig4 = make_component_signature<Position, const Velocity>();
    auto sig5 = make_component_signature<const Position, const Velocity>();
    
    EXPECT_EQ(sig1, sig3) << "const qualifiers should not affect signature";
    EXPECT_EQ(sig1, sig4) << "const qualifiers should not affect signature";
    EXPECT_EQ(sig1, sig5) << "const qualifiers should not affect signature";
}

TEST_F(HephaestusOptimizedSignatureTest, ComponentSignatureHashPerformance) {
    using namespace atlas::hephaestus;
    
    // Test that signature hashing is consistent
    auto sig1 = make_component_signature<Position, Velocity>();
    auto sig2 = make_component_signature<Position, Velocity>();
    
    ComponentSignatureHash hasher;
    EXPECT_EQ(hasher(sig1), hasher(sig2)) << "Equal signatures should have equal hashes";
    
    // Test that different signatures have different hashes (basic collision test)
    auto sig3 = make_component_signature<Position, Health>();
    EXPECT_NE(hasher(sig1), hasher(sig3)) << "Different signatures should have different hashes";
}

TEST_F(HephaestusOptimizedSignatureTest, OptimizedArchetypeMapCompatibility) {
    using namespace atlas::hephaestus;
    
    // Test that OptimizedArchetypeMap can be created and used
    OptimizedArchetypeMap map;
    
    auto sig1 = make_component_signature<Position>();
    auto sig2 = make_component_signature<Position, Velocity>();
    
    // Test insertion and lookup
    map[sig1] = std::make_unique<Archetype>();
    map[sig2] = std::make_unique<Archetype>();
    
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(sig1));
    EXPECT_TRUE(map.contains(sig2));
    
    // Test that different signatures map to different entries
    auto sig3 = make_component_signature<Health>();
    EXPECT_FALSE(map.contains(sig3));
}

TEST_F(HephaestusOptimizedSignatureTest, PerformanceComparison) {
    using namespace atlas::hephaestus;
    using namespace std::chrono;
    
    const int NUM_ITERATIONS = 10000;
    
    // Test legacy signature system performance
    auto start_legacy = high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        auto legacy_sig = make_component_type_signature<Position, Velocity, Health>();
        TypeIndexVectorHash hasher;
        volatile auto hash = hasher(legacy_sig); // volatile to prevent optimization
        (void)hash; // Avoid unused variable warning
    }
    auto end_legacy = high_resolution_clock::now();
    auto legacy_duration = duration_cast<microseconds>(end_legacy - start_legacy);
    
    // Test optimized signature system performance
    auto start_optimized = high_resolution_clock::now();
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        auto optimized_sig = make_component_signature<Position, Velocity, Health>();
        ComponentSignatureHash hasher;
        volatile auto hash = hasher(optimized_sig); // volatile to prevent optimization
        (void)hash; // Avoid unused variable warning
    }
    auto end_optimized = high_resolution_clock::now();
    auto optimized_duration = duration_cast<microseconds>(end_optimized - start_optimized);
    
    std::cout << "Legacy signature system: " << legacy_duration.count() << " microseconds\n";
    std::cout << "Optimized signature system: " << optimized_duration.count() << " microseconds\n";
    
    // The optimized version should be significantly faster
    // We expect at least 2x improvement, but allow for some variance
    EXPECT_LT(optimized_duration.count() * 2, legacy_duration.count()) 
        << "Optimized system should be at least 2x faster than legacy system";
}

TEST_F(HephaestusOptimizedSignatureTest, MemoryFootprintComparison) {
    using namespace atlas::hephaestus;
    
    // Test memory footprint
    auto legacy_sig = make_component_type_signature<Position, Velocity, Health>();
    auto optimized_sig = make_component_signature<Position, Velocity, Health>();
    
    // Legacy signature uses a vector which has heap allocation
    size_t legacy_size = sizeof(legacy_sig) + legacy_sig.size() * sizeof(std::type_index);
    
    // Optimized signature is just a single uint64_t
    size_t optimized_size = sizeof(optimized_sig);
    
    std::cout << "Legacy signature memory footprint: " << legacy_size << " bytes\n";
    std::cout << "Optimized signature memory footprint: " << optimized_size << " bytes\n";
    
    // The optimized version should use significantly less memory
    EXPECT_LT(optimized_size, legacy_size) 
        << "Optimized signature should use less memory than legacy signature";
    
    // For 3 components, we expect significant savings
    EXPECT_LT(optimized_size * 3, legacy_size) 
        << "Optimized signature should use at least 3x less memory";
}

// Demonstration of complete drop-in replacement
TEST_F(HephaestusOptimizedSignatureTest, DropInReplacementDemo) {
    using namespace atlas::hephaestus;
    
    // This test demonstrates how the optimized system could be used as a complete drop-in replacement
    // In a real integration, you would include "hephaestus/OptimizedECS.hpp" instead of the legacy headers
    
    // Create optimized archetype map
    OptimizedArchetypeMap optimized_archetypes;
    
    // Use the optimized signature generation
    auto entity_signature = make_component_signature<Position, Velocity>();
    
    // Add archetype to the map
    optimized_archetypes[entity_signature] = std::make_unique<Archetype>();
    
    // Test lookup performance
    EXPECT_TRUE(optimized_archetypes.contains(entity_signature));
    EXPECT_EQ(optimized_archetypes.size(), 1);
    
    // Add more entities with different signatures
    auto health_signature = make_component_signature<Position, Health>();
    auto full_signature = make_component_signature<Position, Velocity, Health>();
    
    optimized_archetypes[health_signature] = std::make_unique<Archetype>();
    optimized_archetypes[full_signature] = std::make_unique<Archetype>();
    
    EXPECT_EQ(optimized_archetypes.size(), 3);
    EXPECT_TRUE(optimized_archetypes.contains(health_signature));
    EXPECT_TRUE(optimized_archetypes.contains(full_signature));
    
    // Demonstrate signature operations
    EXPECT_TRUE(entity_signature.is_subset_of(full_signature));
    EXPECT_TRUE(health_signature.is_subset_of(full_signature));
    EXPECT_TRUE(entity_signature.intersects_with(full_signature));
    EXPECT_TRUE(entity_signature.intersects_with(health_signature)); // Both have Position component
    
    // Test signatures that don't intersect
    auto velocity_only = make_component_signature<Velocity>();
    auto health_only = make_component_signature<Health>();
    EXPECT_FALSE(velocity_only.intersects_with(health_only)); // No shared components
    
    std::cout << "Drop-in replacement demo: Successfully managed " 
              << optimized_archetypes.size() << " archetypes with optimized signatures\n";
}
