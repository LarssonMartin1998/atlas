#include <gtest/gtest.h>
#include <typeindex>
#include <vector>

#include "hephaestus/Utils.hpp"
#include "hephaestus/Component.hpp"

namespace atlas::hephaestus {

// Simple test components
struct Position : public Component<Position> {
    float x, y, z;
};

struct Velocity : public Component<Velocity> {
    float dx, dy, dz;
};

struct Health : public Component<Health> {
    int hp;
};

class HephaestusConstTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test component access signatures
        non_const_pos_vel = make_component_access_signature<Position&, Velocity&>();
        const_pos_vel = make_component_access_signature<const Position&, const Velocity&>();
        mixed_const_pos_vel = make_component_access_signature<const Position&, Velocity&>();
        non_const_health = make_component_access_signature<Health&>();
        const_health = make_component_access_signature<const Health&>();
    }

    std::vector<ComponentAccess> non_const_pos_vel;
    std::vector<ComponentAccess> const_pos_vel;
    std::vector<ComponentAccess> mixed_const_pos_vel;
    std::vector<ComponentAccess> non_const_health;
    std::vector<ComponentAccess> const_health;
};

// Test ComponentAccess structure
TEST_F(HephaestusConstTest, ComponentAccessStructure) {
    EXPECT_EQ(non_const_pos_vel.size(), 2);
    EXPECT_EQ(const_pos_vel.size(), 2);
    EXPECT_EQ(mixed_const_pos_vel.size(), 2);
    
    // Check that const information is correctly preserved
    for (const auto& access : const_pos_vel) {
        EXPECT_TRUE(access.is_const);
    }
    
    for (const auto& access : non_const_pos_vel) {
        EXPECT_FALSE(access.is_const);
    }
    
    // Check mixed case
    bool found_const = false;
    bool found_non_const = false;
    for (const auto& access : mixed_const_pos_vel) {
        if (access.is_const) {
            found_const = true;
        } else {
            found_non_const = true;
        }
    }
    EXPECT_TRUE(found_const);
    EXPECT_TRUE(found_non_const);
}

// Test that two const-only systems don't conflict
TEST_F(HephaestusConstTest, ConstOnlySystemsNoConflict) {
    EXPECT_FALSE(are_access_signatures_overlapping(const_pos_vel, const_pos_vel));
    EXPECT_FALSE(are_access_signatures_overlapping(const_pos_vel, const_health));
}

// Test that systems with write access conflict with any other access to same component
TEST_F(HephaestusConstTest, WriteAccessConflicts) {
    // Non-const (write) vs const (read) - should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(non_const_pos_vel, const_pos_vel));
    
    // Non-const (write) vs non-const (write) - should conflict
    EXPECT_TRUE(are_access_signatures_overlapping(non_const_pos_vel, non_const_pos_vel));
    
    // Mixed const/non-const vs const - should conflict (due to write component)
    EXPECT_TRUE(are_access_signatures_overlapping(mixed_const_pos_vel, const_pos_vel));
}

// Test that systems with non-overlapping components don't conflict
TEST_F(HephaestusConstTest, NonOverlappingComponentsNoConflict) {
    EXPECT_FALSE(are_access_signatures_overlapping(non_const_pos_vel, non_const_health));
    EXPECT_FALSE(are_access_signatures_overlapping(const_pos_vel, const_health));
    EXPECT_FALSE(are_access_signatures_overlapping(mixed_const_pos_vel, const_health));
}

// Test edge cases
TEST_F(HephaestusConstTest, EdgeCases) {
    std::vector<ComponentAccess> empty;
    
    // Empty vs anything should not conflict
    EXPECT_FALSE(are_access_signatures_overlapping(empty, const_pos_vel));
    EXPECT_FALSE(are_access_signatures_overlapping(const_pos_vel, empty));
    EXPECT_FALSE(are_access_signatures_overlapping(empty, empty));
}

// Test sorting behavior
TEST_F(HephaestusConstTest, SortingBehavior) {
    // Create unsorted signature
    std::vector<ComponentAccess> unsorted = {
        {std::type_index(typeid(Velocity)), true},
        {std::type_index(typeid(Position)), false},
        {std::type_index(typeid(Position)), true}
    };
    
    std::sort(unsorted.begin(), unsorted.end());
    
    // Should be sorted by type first, then by const-ness (false < true)
    EXPECT_EQ(unsorted[0].type, std::type_index(typeid(Position)));
    EXPECT_FALSE(unsorted[0].is_const);
    
    EXPECT_EQ(unsorted[1].type, std::type_index(typeid(Position)));
    EXPECT_TRUE(unsorted[1].is_const);
    
    EXPECT_EQ(unsorted[2].type, std::type_index(typeid(Velocity)));
    EXPECT_TRUE(unsorted[2].is_const);
}

} // namespace atlas::hephaestus