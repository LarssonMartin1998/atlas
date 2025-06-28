#include <gtest/gtest.h>

#include "hephaestus/Component.hpp"
#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {

// Test components
struct Position : public Component<Position> {
    float x = 0.F, y = 0.F, z = 0.F;
};

struct Velocity : public Component<Velocity> {
    float dx = 1.F, dy = 1.F, dz = 1.F;
};

struct Health : public Component<Health> {
    int hp = 1;
};

class ParallelExecutionTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup test data
    }
};

// Test const-aware signature generation for parallel execution scenarios
TEST_F(ParallelExecutionTest, ConstAwareSignatureGeneration) {
    // Create signatures with different const patterns
    auto read_only_position = make_component_access_signature<const Position&>();
    auto write_position = make_component_access_signature<Position&>();
    auto read_only_velocity = make_component_access_signature<const Velocity&>();
    auto write_velocity = make_component_access_signature<Velocity&>();
    auto mixed_access = make_component_access_signature<const Position&, Velocity&>();

    // Two read-only accesses should NOT conflict (can run in parallel)
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_position, read_only_position));

    // Read-only vs write should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(read_only_position, write_position));
    EXPECT_TRUE(are_access_signatures_overlapping(write_position, read_only_position));

    // Write vs write should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(write_position, write_position));

    // Different components should not conflict regardless of const-ness (can run in parallel)
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_position, read_only_velocity));
    EXPECT_FALSE(are_access_signatures_overlapping(write_position, write_velocity));
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_position, write_velocity));

    // Mixed access should conflict with read access to the written component
    EXPECT_TRUE(are_access_signatures_overlapping(mixed_access, read_only_velocity));
    // But not with read access to the read-only component
    EXPECT_FALSE(are_access_signatures_overlapping(mixed_access, read_only_position));
}

// Test multiple read-only systems accessing the same components (should allow parallel execution)
TEST_F(ParallelExecutionTest, MultipleReadOnlySystemsCanRunInParallel) {
    // Create multiple read-only signatures for the same components
    auto read_system1 = make_component_access_signature<const Position&, const Velocity&>();
    auto read_system2 = make_component_access_signature<const Position&, const Velocity&>();
    auto read_system3 = make_component_access_signature<const Position&>();
    auto read_system4 = make_component_access_signature<const Velocity&>();

    // All combinations should be able to run in parallel (no conflicts)
    EXPECT_FALSE(are_access_signatures_overlapping(read_system1, read_system2));
    EXPECT_FALSE(are_access_signatures_overlapping(read_system1, read_system3));
    EXPECT_FALSE(are_access_signatures_overlapping(read_system1, read_system4));
    EXPECT_FALSE(are_access_signatures_overlapping(read_system2, read_system3));
    EXPECT_FALSE(are_access_signatures_overlapping(read_system2, read_system4));
    EXPECT_FALSE(are_access_signatures_overlapping(read_system3, read_system4));
}

// Test write systems causing conflicts (preventing parallel execution)
TEST_F(ParallelExecutionTest, WriteSystemsCauseConflicts) {
    auto write_position = make_component_access_signature<Position&>();
    auto write_position2 = make_component_access_signature<Position&>();
    auto read_position = make_component_access_signature<const Position&>();
    auto mixed_write_position = make_component_access_signature<Position&, const Velocity&>();

    // Any two systems that write to the same component should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(write_position, write_position2));

    // Write vs read of same component should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(write_position, read_position));
    EXPECT_TRUE(are_access_signatures_overlapping(read_position, write_position));

    // Mixed write system should conflict with anything accessing the written component
    EXPECT_TRUE(are_access_signatures_overlapping(mixed_write_position, write_position));
    EXPECT_TRUE(are_access_signatures_overlapping(mixed_write_position, read_position));
}

// Test that systems accessing completely different components can run in parallel
TEST_F(ParallelExecutionTest, NonOverlappingSystemsCanRunInParallel) {
    auto position_system = make_component_access_signature<Position&>();
    auto velocity_system = make_component_access_signature<Velocity&>();
    auto health_system = make_component_access_signature<Health&>();
    auto position_velocity_system = make_component_access_signature<Position&, Velocity&>();

    // Systems accessing completely different components should not conflict
    EXPECT_FALSE(are_access_signatures_overlapping(position_system, health_system));
    EXPECT_FALSE(are_access_signatures_overlapping(velocity_system, health_system));

    // But systems accessing overlapping components should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(position_system, position_velocity_system));
    EXPECT_TRUE(are_access_signatures_overlapping(velocity_system, position_velocity_system));
}

// Test complex parallel execution scenarios
TEST_F(ParallelExecutionTest, ComplexParallelExecutionScenarios) {
    // Scenario: Physics simulation with rendering and audio systems

    // Physics systems (write access)
    auto physics_movement = make_component_access_signature<Position&, const Velocity&>();
    auto physics_collision = make_component_access_signature<Position&, Health&>();

    // Rendering systems (read-only access)
    auto render_objects = make_component_access_signature<const Position&, const Velocity&>();
    auto render_ui = make_component_access_signature<const Health&>();

    // Audio systems (read-only access)
    auto audio_effects = make_component_access_signature<const Position&>();
    auto audio_music = make_component_access_signature<const Health&>();

    // Physics systems should conflict with each other (write access)
    EXPECT_TRUE(are_access_signatures_overlapping(physics_movement, physics_collision));

    // Physics systems should conflict with rendering that reads the same components
    EXPECT_TRUE(are_access_signatures_overlapping(physics_movement, render_objects));
    EXPECT_TRUE(are_access_signatures_overlapping(physics_collision, render_objects));
    EXPECT_TRUE(are_access_signatures_overlapping(physics_collision, render_ui));

    // Rendering systems should be able to run in parallel with each other
    EXPECT_FALSE(are_access_signatures_overlapping(render_objects, render_ui));

    // Audio systems should be able to run in parallel with each other
    EXPECT_FALSE(are_access_signatures_overlapping(audio_effects, audio_music));

    // Audio and rendering systems should be able to run in parallel (all read-only)
    EXPECT_FALSE(are_access_signatures_overlapping(render_objects, audio_effects));
    EXPECT_FALSE(are_access_signatures_overlapping(render_ui, audio_music));

    // But audio systems conflict with physics that write to the same components
    EXPECT_TRUE(are_access_signatures_overlapping(physics_movement, audio_effects));
    EXPECT_TRUE(are_access_signatures_overlapping(physics_collision, audio_effects));
    EXPECT_TRUE(are_access_signatures_overlapping(physics_collision, audio_music));
}

// Test that the parallel execution benefits are correctly detected
TEST_F(ParallelExecutionTest, ParallelExecutionBenefitDetection) {
    // Simulate a scenario with 5 systems and check expected parallelism

    // 3 read-only systems (should be able to run in parallel)
    auto read_system1 = make_component_access_signature<const Position&>();
    auto read_system2 = make_component_access_signature<const Position&>();
    auto read_system3 = make_component_access_signature<const Velocity&>();

    // 2 write systems (should conflict with everything)
    auto write_system1 = make_component_access_signature<Position&>();
    auto write_system2 = make_component_access_signature<Velocity&>();

    std::vector<std::vector<ComponentAccess>> systems =
        {read_system1, read_system2, read_system3, write_system1, write_system2};

    // Count conflicts for each system (simulate dependency graph building)
    std::vector<int> conflicts(systems.size(), 0);

    for (size_t i = 0; i < systems.size(); ++i) {
        for (size_t j = i + 1; j < systems.size(); ++j) {
            if (are_access_signatures_overlapping(systems[i], systems[j])) {
                conflicts[i]++;
                conflicts[j]++;
            }
        }
    }

    // Read-only systems accessing different components should have fewer conflicts
    EXPECT_EQ(conflicts[0], 1); // read_system1 conflicts with write_system1
    EXPECT_EQ(conflicts[1], 1); // read_system2 conflicts with write_system1
    EXPECT_EQ(conflicts[2], 1); // read_system3 conflicts with write_system2

    // Write systems should have more conflicts
    EXPECT_EQ(conflicts[3], 2); // write_system1 conflicts with read_system1 and read_system2
    EXPECT_EQ(conflicts[4], 1); // write_system2 conflicts with read_system3

    // This means read systems can have higher concurrent_systems_estimate
    // leading to better parallel utilization
    auto total_systems = systems.size();
    for (size_t i = 0; i < 3; ++i) { // read-only systems
        auto concurrent_estimate = std::max<std::size_t>(1, total_systems - conflicts[i]);
        EXPECT_GE(concurrent_estimate, 4)
            << "Read-only system " << i << " should have high concurrency estimate";
    }
}

} // namespace atlas::hephaestus
