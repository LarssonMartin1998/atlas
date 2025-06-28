#include <gtest/gtest.h>

#include "hephaestus/Component.hpp"
#include "hephaestus/Hephaestus.hpp"

namespace atlas::hephaestus {

// Test components
struct Position : public Component<Position> {
    float x = 0.F, y = 0.F, z = 0.F;
};

struct Velocity : public Component<Velocity> {
    float dx = 0.F, dy = 0.F, dz = 0.F;
};

class EndToEndConstTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // For this test, we'll verify that the dependency graph logic works
    }
};

// Test that the new dependency graph logic correctly handles const vs non-const
TEST_F(EndToEndConstTest, DependencyGraphLogic) {
    // Create SystemNodes with different access patterns
    SystemNode read_only_node;
    read_only_node.component_access_dependencies = make_component_access_signature<
        const Position&,
        const Velocity&>();

    SystemNode write_position_node;
    write_position_node.component_access_dependencies = make_component_access_signature<
        Position&,
        const Velocity&>();

    SystemNode write_velocity_node;
    write_velocity_node.component_access_dependencies = make_component_access_signature<
        const Position&,
        Velocity&>();

    SystemNode write_both_node;
    write_both_node
        .component_access_dependencies = make_component_access_signature<Position&, Velocity&>();

    // Test overlap detection using the same logic as build_systems_dependency_graph
    auto are_nodes_conflicting = [](const SystemNode& node, const SystemNode& other) {
        return are_access_signatures_overlapping(
            node.component_access_dependencies,
            other.component_access_dependencies
        );
    };

    // Two read-only systems should NOT conflict
    EXPECT_FALSE(are_nodes_conflicting(read_only_node, read_only_node));

    // Read-only vs write should conflict
    EXPECT_TRUE(are_nodes_conflicting(read_only_node, write_position_node));
    EXPECT_TRUE(are_nodes_conflicting(read_only_node, write_velocity_node));
    EXPECT_TRUE(are_nodes_conflicting(read_only_node, write_both_node));

    // Systems writing to different components should still CONFLICT if they both access common
    // components (even if one reads what the other writes)
    EXPECT_TRUE(are_nodes_conflicting(write_position_node, write_velocity_node));

    // Systems writing to overlapping components should conflict
    EXPECT_TRUE(are_nodes_conflicting(write_position_node, write_both_node));
    EXPECT_TRUE(are_nodes_conflicting(write_velocity_node, write_both_node));
    EXPECT_TRUE(are_nodes_conflicting(write_both_node, write_both_node));
}

// Test the complete flow: type preservation through the signature creation
TEST_F(EndToEndConstTest, CompleteConstPreservationFlow) {
    // Verify that std::is_const_v works correctly on our test types
    EXPECT_TRUE(std::is_const_v<std::remove_reference_t<const Position&>>);
    EXPECT_FALSE(std::is_const_v<std::remove_reference_t<Position&>>);

    // Test the complete signature creation pipeline
    auto mixed_signature = make_component_access_signature<
        const Position&,
        Velocity&,
        const Velocity&>();

    EXPECT_EQ(mixed_signature.size(), 3);

    // Verify sorting and correct const detection
    int const_count = 0;
    int non_const_count = 0;

    for (const auto& access : mixed_signature) {
        if (access.is_read_only) {
            const_count++;
        } else {
            non_const_count++;
        }
    }

    EXPECT_EQ(const_count, 2) << "Should have 2 const accesses (const Position&, const Velocity&)";
    EXPECT_EQ(non_const_count, 1) << "Should have 1 non-const access (Velocity&)";
}

// Demonstrate that const-only systems can run in parallel
TEST_F(EndToEndConstTest, ParallelReadOnlySystemsDemo) {
    // Create multiple read-only signatures
    auto reader1_sig = make_component_access_signature<const Position&>();
    auto reader2_sig = make_component_access_signature<const Position&>();
    auto reader3_sig = make_component_access_signature<const Position&, const Velocity&>();

    // None of these should conflict with each other
    EXPECT_FALSE(are_access_signatures_overlapping(reader1_sig, reader2_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(reader1_sig, reader3_sig));
    EXPECT_FALSE(are_access_signatures_overlapping(reader2_sig, reader3_sig));

    // But they should conflict with a writer
    auto writer_sig = make_component_access_signature<Position&>();
    EXPECT_TRUE(are_access_signatures_overlapping(reader1_sig, writer_sig));
    EXPECT_TRUE(are_access_signatures_overlapping(reader2_sig, writer_sig));
    EXPECT_TRUE(are_access_signatures_overlapping(reader3_sig, writer_sig));
}

// Test advanced scenarios with multiple component types
TEST_F(EndToEndConstTest, AdvancedConstScenarios) {
    // Test scenarios that demonstrate the benefits of const-aware scheduling

    // Scenario 1: Two systems that both only READ the same components - should NOT conflict
    auto render_system_sig = make_component_access_signature<const Position&, const Velocity&>();
    auto audio_system_sig = make_component_access_signature<const Position&, const Velocity&>();

    // Both systems only read - no conflict
    EXPECT_FALSE(are_access_signatures_overlapping(render_system_sig, audio_system_sig));

    // Scenario 2: System that reads Position but writes Velocity vs system that only reads
    // different components
    auto motion_system_sig = make_component_access_signature<const Position&, Velocity&>();
    auto position_only_reader_sig = make_component_access_signature<const Position&>();

    // These should conflict because motion_system writes Velocity while position_reader reads
    // Position Actually, let me think about this... motion_system reads Position, position_reader
    // reads Position Since both only read Position, they shouldn't conflict on Position access But
    // motion_system also accesses Velocity (write), position_reader doesn't access Velocity So they
    // should NOT conflict since they don't have overlapping write access
    EXPECT_FALSE(are_access_signatures_overlapping(motion_system_sig, position_only_reader_sig));

    // Scenario 3: Systems accessing completely different component sets
    auto position_writer_sig = make_component_access_signature<Position&>();
    auto velocity_reader_sig = make_component_access_signature<const Velocity&>();

    // No shared components - no conflict
    EXPECT_FALSE(are_access_signatures_overlapping(position_writer_sig, velocity_reader_sig));

    // Scenario 4: Write vs write on the same component - should conflict
    auto position_writer1_sig = make_component_access_signature<Position&>();
    auto position_writer2_sig = make_component_access_signature<Position&>();

    EXPECT_TRUE(are_access_signatures_overlapping(position_writer1_sig, position_writer2_sig));
}

} // namespace atlas::hephaestus
