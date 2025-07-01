#include <gtest/gtest.h>
#include <atlas/core/time/Timer.hpp>
#include <atlas/core/time/EngineClock.hpp>
#include <thread>
#include <chrono>

namespace atlas::core {

class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test Timer functionality
TEST_F(TimerTest, ElapsedTimeInitiallyZero) {
    Timer timer;
    // Timer should have minimal elapsed time initially (microseconds level)
    auto elapsed = timer.elapsed();
    EXPECT_GE(elapsed, 0.0);
    EXPECT_LT(elapsed, 0.001); // Should be less than 1ms for a fresh timer
}

TEST_F(TimerTest, ElapsedTimeIncreasesOverTime) {
    Timer timer;
    
    // Small delay
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto elapsed1 = timer.elapsed();
    EXPECT_GT(elapsed1, 0.005); // Should be at least 5ms
    
    // Another small delay
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto elapsed2 = timer.elapsed();
    EXPECT_GT(elapsed2, elapsed1); // Should have increased
    EXPECT_GT(elapsed2, 0.015); // Should be at least 15ms total
}

TEST_F(TimerTest, ResetResetsElapsedTime) {
    Timer timer;
    
    // Wait some time
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    auto elapsed_before_reset = timer.elapsed();
    EXPECT_GT(elapsed_before_reset, 0.01); // Should have some time
    
    // Reset timer
    timer.reset();
    
    auto elapsed_after_reset = timer.elapsed();
    EXPECT_LT(elapsed_after_reset, elapsed_before_reset); // Should be much smaller
    EXPECT_LT(elapsed_after_reset, 0.001); // Should be very close to zero
}

TEST_F(TimerTest, MultipleResets) {
    Timer timer;
    
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        timer.reset();
        auto elapsed = timer.elapsed();
        EXPECT_LT(elapsed, 0.001); // Should be very small after each reset
    }
}

// Test EngineClock functionality
class EngineClockTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
    
    EngineClock clock;
};

TEST_F(EngineClockTest, InitialStateZeroDeltaTime) {
    // Initially, delta time should be zero since no update has occurred
    EXPECT_EQ(clock.get_delta_time(), 0.0);
    
    // Total time should be very small (just created)
    auto total_time = clock.get_total_time();
    EXPECT_GE(total_time, 0.0);
    EXPECT_LT(total_time, 0.001);
}

TEST_F(EngineClockTest, TotalTimeIncreasesOverTime) {
    auto initial_total = clock.get_total_time();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto later_total = clock.get_total_time();
    EXPECT_GT(later_total, initial_total);
    EXPECT_GT(later_total, 0.005); // Should be at least 5ms
}

TEST_F(EngineClockTest, UpdateDeltaTimeSetsCorrectDeltaTime) {
    // First update - should capture some initial time
    clock.update_delta_time();
    auto first_delta = clock.get_delta_time();
    EXPECT_GE(first_delta, 0.0);
    EXPECT_LT(first_delta, 0.001); // Should be very small for first update
    
    // Wait some time
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // Second update - should capture the waited time
    clock.update_delta_time();
    auto second_delta = clock.get_delta_time();
    EXPECT_GT(second_delta, first_delta);
    EXPECT_GT(second_delta, 0.015); // Should be at least 15ms
    EXPECT_LT(second_delta, 0.030); // But shouldn't be too much more than 20ms
}

TEST_F(EngineClockTest, ConsecutiveDeltaTimeUpdates) {
    std::vector<double> delta_times;
    
    // Collect several delta time measurements
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        clock.update_delta_time();
        delta_times.push_back(clock.get_delta_time());
    }
    
    // Each delta time should be reasonable (around 10ms)
    for (double delta : delta_times) {
        EXPECT_GT(delta, 0.005); // At least 5ms
        EXPECT_LT(delta, 0.020); // Less than 20ms (allowing for timing variations)
    }
}

TEST_F(EngineClockTest, TotalTimeIsIndependentOfDeltaTimeUpdates) {
    auto initial_total = clock.get_total_time();
    
    // Multiple delta time updates
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        clock.update_delta_time();
    }
    
    auto final_total = clock.get_total_time();
    
    // Total time should have increased by approximately the sum of all waits
    EXPECT_GT(final_total, initial_total + 0.010); // At least 10ms more
    
    // Total time should be independent of delta time operations
    EXPECT_GE(final_total, 0.015); // Should be at least 15ms total
}

TEST_F(EngineClockTest, SimulateGameLoop) {
    // Simulate a typical game loop pattern
    const int frame_count = 10;
    std::vector<double> frame_times;
    
    for (int frame = 0; frame < frame_count; ++frame) {
        // Simulate frame processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // ~125 FPS target
        
        clock.update_delta_time();
        double delta = clock.get_delta_time();
        frame_times.push_back(delta);
        
        // Each frame delta should be reasonable
        EXPECT_GT(delta, 0.005); // At least 5ms
        EXPECT_LT(delta, 0.015); // Less than 15ms
    }
    
    // Total time should be approximately the sum of all frame times
    double total_delta_sum = 0.0;
    for (double dt : frame_times) {
        total_delta_sum += dt;
    }
    
    auto total_time = clock.get_total_time();
    EXPECT_GT(total_time, total_delta_sum * 0.8); // Allow some variance
    EXPECT_LT(total_time, total_delta_sum * 1.2); // But not too much
}

} // namespace atlas::core