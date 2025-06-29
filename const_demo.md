# Atlas ECS Const-Aware System Demo

This demonstrates the new const-aware component access functionality in the Hephaestus ECS module.

## Problem Solved

Previously, systems could not specify whether they only read (const access) or write to components. This led to conservative scheduling where any two systems accessing the same component types would be considered conflicting, even if both only read the data.

## New Functionality

Now systems can specify const access patterns:

```cpp
// Read-only system - can run in parallel with other read-only systems
hephaestus.create_system([](Engine& engine, std::tuple<const Transform&, const Velocity&>& components) {
    const auto& [transform, velocity] = components;
    // Only read data - no modifications
    render_object(transform.position, velocity.direction);
});

// Write system - cannot run in parallel with systems accessing the same components
hephaestus.create_system([](Engine& engine, std::tuple<Transform&, const Velocity&>& components) {
    auto& [transform, velocity] = components;
    // Read velocity, write transform
    transform.position += velocity.delta * dt;
});

// Mixed system - reads some components, writes others
hephaestus.create_system([](Engine& engine, std::tuple<const Transform&, Health&>& components) {
    const auto& [transform, health] = components;
    // Read transform, modify health based on position
    if (transform.position.y < -100.0f) {
        health.hp -= 10; // Fall damage
    }
});
```

## Benefits

1. **Better Parallelism**: Multiple read-only systems can execute simultaneously
2. **Type Safety**: Const correctness enforced at compile time
3. **Performance**: Improved ECS scheduling reduces unnecessary synchronization
4. **Clear Intent**: Code clearly expresses whether components are read or modified

## Technical Implementation

- New `ComponentAccess` structure tracks both type and const-ness
- Enhanced dependency graph considers read vs write access patterns
- Systems with only const access to overlapping components can run in parallel
- Backward compatible with existing non-const system declarations

## Test Results

All tests pass, demonstrating:
- Const information is correctly preserved through type deduction
- Overlap detection properly handles const vs non-const access
- Systems can be created with various const patterns
- Dependency graph scheduling works correctly with const-aware signatures