#include <gtest/gtest.h>

#include "hephaestus/Hephaestus.hpp"
#include "hephaestus/Component.hpp"
#include "core/Game.hpp"

namespace atlas::hephaestus {

// Test components
struct Transform : public Component<Transform> {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct Velocity : public Component<Velocity> {
    float dx = 0.0f, dy = 0.0f, dz = 0.0f;
};

class SystemConstIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple Hephaestus instance for testing
        // Note: This is a simplified test that doesn't require full Game setup
    }
};

// Test that const-aware function trait deduction works
TEST_F(SystemConstIntegrationTest, FunctionTraitsPreserveConst) {
    // Test that we can create const-aware signatures
    auto const_signature = make_component_access_signature<const Transform&, const Velocity&>();
    auto non_const_signature = make_component_access_signature<Transform&, Velocity&>();
    auto mixed_signature = make_component_access_signature<const Transform&, Velocity&>();
    
    EXPECT_EQ(const_signature.size(), 2);
    EXPECT_EQ(non_const_signature.size(), 2);
    EXPECT_EQ(mixed_signature.size(), 2);
    
    // Verify const information is preserved
    for (const auto& access : const_signature) {
        EXPECT_TRUE(access.is_const);
    }
    
    for (const auto& access : non_const_signature) {
        EXPECT_FALSE(access.is_const);
    }
    
    // For mixed signature, check we have both const and non-const
    bool has_const = false;
    bool has_non_const = false;
    for (const auto& access : mixed_signature) {
        if (access.is_const) {
            has_const = true;
        } else {
            has_non_const = true;
        }
    }
    EXPECT_TRUE(has_const);
    EXPECT_TRUE(has_non_const);
}

// Test that system creation compiles with various const patterns
TEST_F(SystemConstIntegrationTest, SystemCreationCompiles) {
    // This test verifies that the type deduction and signature creation
    // works correctly for different const patterns
    
    // Test signature creation for different const patterns
    auto sig1 = make_component_access_signature<const Transform&>();
    auto sig2 = make_component_access_signature<Transform&>();
    auto sig3 = make_component_access_signature<const Transform&, Velocity&>();
    
    EXPECT_EQ(sig1.size(), 1);
    EXPECT_TRUE(sig1[0].is_const);
    
    EXPECT_EQ(sig2.size(), 1);
    EXPECT_FALSE(sig2[0].is_const);
    
    EXPECT_EQ(sig3.size(), 2);
    // Verify mixed const/non-const
    bool found_const_transform = false;
    bool found_non_const_velocity = false;
    for (const auto& access : sig3) {
        if (access.type == std::type_index(typeid(Transform)) && access.is_const) {
            found_const_transform = true;
        }
        if (access.type == std::type_index(typeid(Velocity)) && !access.is_const) {
            found_non_const_velocity = true;
        }
    }
    EXPECT_TRUE(found_const_transform);
    EXPECT_TRUE(found_non_const_velocity);
}

// Test that overlapping logic works correctly with const awareness
TEST_F(SystemConstIntegrationTest, ConstAwareOverlapDetection) {
    // Create signatures with different const patterns
    auto read_only_transform = make_component_access_signature<const Transform&>();
    auto write_transform = make_component_access_signature<Transform&>();
    auto read_only_velocity = make_component_access_signature<const Velocity&>();
    auto write_velocity = make_component_access_signature<Velocity&>();
    
    // Two read-only accesses should NOT conflict
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_transform, read_only_transform));
    
    // Read-only vs write should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(read_only_transform, write_transform));
    EXPECT_TRUE(are_access_signatures_overlapping(write_transform, read_only_transform));
    
    // Write vs write should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(write_transform, write_transform));
    
    // Different components should not conflict regardless of const-ness
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_transform, read_only_velocity));
    EXPECT_FALSE(are_access_signatures_overlapping(write_transform, write_velocity));
    EXPECT_FALSE(are_access_signatures_overlapping(read_only_transform, write_velocity));
}

} // namespace atlas::hephaestus