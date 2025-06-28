#include <gtest/gtest.h>
#include <hephaestus/Utils.hpp>
#include <hephaestus/Component.hpp>
#include <atlas/core/Engine.hpp>
#include <atlas/core/IGame.hpp>
#include <hephaestus/Hephaestus.hpp>
#include <chrono>

using namespace atlas;
using namespace atlas::hephaestus;

// Simple test components
struct Position : public atlas::hephaestus::Component<Position> { float x, y; };
struct Velocity : public atlas::hephaestus::Component<Velocity> { float dx, dy; };
struct Health : public atlas::hephaestus::Component<Health> { int value; };

// Global test state
struct TestState {
    int physics_runs = 0;
    int renderer_runs = 0;
    int health_checker_runs = 0;
    std::vector<Position> final_positions;
};
static TestState* test_state = nullptr;

// Mock game class for testing  
class TestGame : public atlas::core::IGame {
public:
    auto pre_start() -> void override {}
    auto start() -> void override {
        auto& hephaestus = engine_ptr->get_module<Hephaestus>();
        
        // Create some entities first
        hephaestus.create_entity(Position{1.0f, 2.0f}, Velocity{0.5f, -0.5f});
        hephaestus.create_entity(Position{3.0f, 4.0f}, Velocity{1.0f, 1.0f});
        hephaestus.create_entity(Position{5.0f, 6.0f}, Health{100});

        if (test_state) {
            // Create a physics system (writes to Position, reads Velocity)
            hephaestus.create_system([](const atlas::core::IEngine& engine, std::tuple<Position&, const Velocity&>& components) {
                auto& [pos, vel] = components;
                pos.x += vel.dx * 0.1f; // Small delta for testing
                pos.y += vel.dy * 0.1f;
                test_state->physics_runs++;
            });

            // Create a renderer system (reads Position and Velocity)
            hephaestus.create_system([](const atlas::core::IEngine& engine, const std::tuple<const Position&, const Velocity&>& components) {
                const auto& [pos, vel] = components;
                // Simulate rendering
                test_state->renderer_runs++;
            });

            // Create a health checker system (reads Health)
            hephaestus.create_system([](const atlas::core::IEngine& engine, std::tuple<const Health&>& components) {
                const auto& [health] = components;
                // Simulate health checking
                test_state->health_checker_runs++;
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
    auto set_engine(atlas::core::IEngine& engine_ref) -> void override { 
        engine_ptr = &engine_ref; 
    }
    auto should_quit() const -> bool override { 
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
    auto pos_it = std::find_if(mixed_signature.begin(), mixed_signature.end(),
        [](const ComponentAccess& access) { return access.type == std::type_index(typeid(Position)); });
    auto vel_it = std::find_if(mixed_signature.begin(), mixed_signature.end(),
        [](const ComponentAccess& access) { return access.type == std::type_index(typeid(Velocity)); });
    
    ASSERT_NE(pos_it, mixed_signature.end());
    ASSERT_NE(vel_it, mixed_signature.end());
    EXPECT_TRUE(pos_it->is_read_only);   // const Position&
    EXPECT_FALSE(vel_it->is_read_only);  // Velocity&
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
    auto has_position = std::find(signature.begin(), signature.end(), std::type_index(typeid(Position))) != signature.end();
    auto has_velocity = std::find(signature.begin(), signature.end(), std::type_index(typeid(Velocity))) != signature.end();
    
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

// Test 5: Extract component types from access signature
TEST_F(HephaestusUtilsTest, ExtractComponentTypes) {
    auto access_sig = make_component_access_signature<const Position&, Velocity&>();
    auto type_sig = extract_component_types(access_sig);
    
    EXPECT_EQ(type_sig.size(), 2);
    auto has_position = std::find(type_sig.begin(), type_sig.end(), std::type_index(typeid(Position))) != type_sig.end();
    auto has_velocity = std::find(type_sig.begin(), type_sig.end(), std::type_index(typeid(Velocity))) != type_sig.end();
    
    EXPECT_TRUE(has_position);
    EXPECT_TRUE(has_velocity);
}

// Test 6: Parallel execution benefits verification
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

// Integration test that actually runs the engine
TEST(HephaestusIntegrationTest, ActualSystemCreationAndExecution) {
    // Set up test state
    TestState state;
    test_state = &state;
    
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
    test_state = nullptr;
}