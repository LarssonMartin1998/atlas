#include <gtest/gtest.h>
#include <atlas/core/Engine.hpp>
#include <hephaestus/Hephaestus.hpp>
#include <hephaestus/Utils.hpp>

using namespace atlas;
using namespace atlas::hephaestus;

// Simple test components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int value; };

class HephaestusIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<core::Engine>();
        engine->initialize();
        hephaestus = &engine->get_module<Hephaestus>();
    }

    void TearDown() override {
        if (engine) {
            engine->shutdown();
        }
    }

    std::unique_ptr<core::Engine> engine;
    Hephaestus* hephaestus = nullptr;
};

// Test 1: Basic const signature generation
TEST_F(HephaestusIntegrationTest, ComponentAccessSignatureGeneration) {
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
TEST_F(HephaestusIntegrationTest, ConstAwareConflictDetection) {
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

// Test 3: Actual system creation and execution
TEST_F(HephaestusIntegrationTest, SystemCreationAndExecution) {
    // Create some entities first
    hephaestus->create_entity(Position{1.0f, 2.0f}, Velocity{0.5f, -0.5f});
    hephaestus->create_entity(Position{3.0f, 4.0f}, Velocity{1.0f, 1.0f});
    hephaestus->create_entity(Position{5.0f, 6.0f}, Health{100});

    // Counter to verify systems actually run
    int physics_runs = 0;
    int renderer_runs = 0;
    int health_checker_runs = 0;

    // Create a physics system (writes to Position, reads Velocity)
    hephaestus->create_system([&physics_runs](const core::IEngine& engine, std::tuple<Position&, const Velocity&>& components) {
        auto& [pos, vel] = components;
        pos.x += vel.dx;
        pos.y += vel.dy;
        physics_runs++;
    });

    // Create a renderer system (reads Position and Velocity)
    hephaestus->create_system([&renderer_runs](const core::IEngine& engine, const std::tuple<const Position&, const Velocity&>& components) {
        const auto& [pos, vel] = components;
        // Simulate rendering
        renderer_runs++;
    });

    // Create a health checker system (reads Health)
    hephaestus->create_system([&health_checker_runs](const core::IEngine& engine, std::tuple<const Health&>& components) {
        const auto& [health] = components;
        // Simulate health checking
        health_checker_runs++;
    });

    // Start the engine (this should trigger system creation)
    engine->start();

    // Execute one frame
    engine->tick();

    // Verify systems actually executed
    EXPECT_EQ(physics_runs, 2) << "Physics system should run on 2 entities with Position+Velocity";
    EXPECT_EQ(renderer_runs, 2) << "Renderer system should run on 2 entities with Position+Velocity";
    EXPECT_EQ(health_checker_runs, 1) << "Health checker should run on 1 entity with Health";
}

// Test 4: Parallel execution benefits (multiple read-only systems)
TEST_F(HephaestusIntegrationTest, ParallelExecutionBenefits) {
    // Create entities
    hephaestus->create_entity(Position{0.0f, 0.0f}, Velocity{1.0f, 1.0f});
    hephaestus->create_entity(Position{10.0f, 10.0f}, Velocity{-1.0f, -1.0f});

    int audio_runs = 0;
    int ui_runs = 0;
    int logger_runs = 0;

    // Create multiple read-only systems that should be able to run in parallel
    hephaestus->create_system([&audio_runs](const core::IEngine& engine, const std::tuple<const Position&>& components) {
        const auto& [pos] = components;
        // Audio positioning
        audio_runs++;
    });

    hephaestus->create_system([&ui_runs](const core::IEngine& engine, const std::tuple<const Position&, const Velocity&>& components) {
        const auto& [pos, vel] = components;
        // UI updates
        ui_runs++;
    });

    hephaestus->create_system([&logger_runs](const core::IEngine& engine, const std::tuple<const Position&>& components) {
        const auto& [pos] = components;
        // Logging
        logger_runs++;
    });

    engine->start();
    engine->tick();

    // All read-only systems should execute
    EXPECT_EQ(audio_runs, 2);
    EXPECT_EQ(ui_runs, 2);
    EXPECT_EQ(logger_runs, 2);

    // The key benefit test: these systems' signatures should not conflict
    auto audio_sig = make_component_access_signature<const Position&>();
    auto ui_sig = make_component_access_signature<const Position&, const Velocity&>();
    auto logger_sig = make_component_access_signature<const Position&>();

    EXPECT_FALSE(are_access_signatures_overlapping(audio_sig, ui_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(audio_sig, logger_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(ui_sig, logger_sig));
}

// Test 5: Type signature generation (for archetype queries)
TEST_F(HephaestusIntegrationTest, TypeSignatureGeneration) {
    auto signature = make_component_type_signature<Position, Velocity>();
    EXPECT_EQ(signature.size(), 2);
    
    // Should contain type indices for Position and Velocity
    auto has_position = std::find(signature.begin(), signature.end(), std::type_index(typeid(Position))) != signature.end();
    auto has_velocity = std::find(signature.begin(), signature.end(), std::type_index(typeid(Velocity))) != signature.end();
    
    EXPECT_TRUE(has_position);
    EXPECT_TRUE(has_velocity);
}

// Test 6: Mixed const/non-const system execution
TEST_F(HephaestusIntegrationTest, MixedConstNonConstExecution) {
    hephaestus->create_entity(Position{0.0f, 0.0f}, Velocity{1.0f, 0.0f}, Health{100});

    int damage_system_runs = 0;

    // System that reads health and modifies position (mixed access)
    hephaestus->create_system([&damage_system_runs](const core::IEngine& engine, std::tuple<Position&, const Health&>& components) {
        auto& [pos, health] = components;
        if (health.value < 50) {
            pos.x += 10.0f; // Knockback effect
        }
        damage_system_runs++;
    });

    engine->start();
    engine->tick();

    EXPECT_EQ(damage_system_runs, 1);
}

// Test 7: Edge case - empty signature
TEST_F(HephaestusIntegrationTest, EdgeCases) {
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