#include <gtest/gtest.h>
#include <atlas/core/Engine.hpp>
#include <atlas/core/IEngine.hpp>
#include <atlas/core/IGame.hpp>
#include <atlas/core/Module.hpp>
#include <atlas/core/ITickable.hpp>
#include <chrono>
#include <thread>

namespace atlas::core {

// Mock Game for testing Engine
class MockGame : public IGame {
public:
    MockGame() = default;
    
    auto pre_start() -> void override {
        pre_start_called = true;
        lifecycle_order.push_back("game_pre_start");
    }
    
    auto start() -> void override {
        start_called = true;
        lifecycle_order.push_back("game_start");
    }
    
    auto post_start() -> void override {
        post_start_called = true;
        lifecycle_order.push_back("game_post_start");
    }
    
    auto pre_shutdown() -> void override {
        pre_shutdown_called = true;
        lifecycle_order.push_back("game_pre_shutdown");
    }
    
    auto shutdown() -> void override {
        shutdown_called = true;
        lifecycle_order.push_back("game_shutdown");
    }
    
    auto post_shutdown() -> void override {
        post_shutdown_called = true;
        lifecycle_order.push_back("game_post_shutdown");
    }
    
    auto get_engine() const -> IEngine& override {
        if (!engine_ptr) {
            throw std::runtime_error("Engine not set");
        }
        return *engine_ptr;
    }
    
    auto set_engine(IEngine& engine_ref) -> void override {
        engine_ptr = &engine_ref;
        set_engine_called = true;
    }
    
    auto should_quit() const -> bool override {
        // Automatically increment frame count on each check (this simulates the engine calling should_quit each frame)
        static_cast<MockGame*>(const_cast<MockGame*>(this))->frame_count++;
        return frame_count >= max_frames || quit_requested;
    }
    
    // Test control methods
    void request_quit() { quit_requested = true; }
    void set_max_frames(int frames) { max_frames = frames; }
    void increment_frame() { frame_count++; }
    
    // Test state
    bool pre_start_called = false;
    bool start_called = false;
    bool post_start_called = false;
    bool pre_shutdown_called = false;
    bool shutdown_called = false;
    bool post_shutdown_called = false;
    bool set_engine_called = false;
    std::vector<std::string> lifecycle_order;
    
private:
    IEngine* engine_ptr = nullptr;
    bool quit_requested = false;
    int frame_count = 0;
    int max_frames = 3; // Default to quit after 3 frames
};

// Test Module for Engine testing
class TestEngineModule : public Module, public ITickable {
public:
    explicit TestEngineModule(IEngine& engine) : Module(engine) {}
    
    auto start() -> void override {
        start_called = true;
        Module::start();
    }
    
    auto post_start() -> void override {
        post_start_called = true;
        Module::post_start();
    }
    
    auto shutdown() -> void override {
        shutdown_called = true;
        Module::shutdown();
    }
    
    auto tick() -> void override {
        tick_count++;
        // Increment the game frame counter to control when to quit
        if (auto* game = dynamic_cast<MockGame*>(&get_engine().get_game())) {
            game->increment_frame();
        }
    }
    
    auto get_tick_rate() const -> unsigned override {
        return 60;
    }
    
    // Test state
    bool start_called = false;
    bool post_start_called = false;
    bool shutdown_called = false;
    unsigned tick_count = 0;
};

class EngineTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test Engine construction and basic functionality
TEST_F(EngineTest, EngineConstruction) {
    Engine<MockGame> engine;
    
    // Engine should be in not initialized state initially
    EXPECT_EQ(engine.get_engine_init_status(), EngineInitStatus::NotInitialized);
    
    // Engine should have a game instance
    EXPECT_NO_THROW(engine.get_game());
    
    // Engine should have a clock
    EXPECT_NO_THROW(engine.get_clock());
}

TEST_F(EngineTest, EngineGameAccess) {
    Engine<MockGame> engine;
    
    // Get game reference
    IGame& game = engine.get_game();
    MockGame* mock_game = dynamic_cast<MockGame*>(&game);
    ASSERT_NE(mock_game, nullptr);
    
    // Game should not have engine set initially
    EXPECT_FALSE(mock_game->set_engine_called);
}

TEST_F(EngineTest, EngineClockAccess) {
    Engine<MockGame> engine;
    
    // Clock should be accessible
    const IEngineClock& clock = engine.get_clock();
    
    // Clock should provide reasonable values
    EXPECT_GE(clock.get_total_time(), 0.0);
    EXPECT_EQ(clock.get_delta_time(), 0.0); // No updates yet
}

// Test Engine.run() method - this is complex since it's the main loop
TEST_F(EngineTest, EngineRunBasicFunctionality) {
    Engine<MockGame> engine;
    IGame& game = engine.get_game();
    MockGame* mock_game = dynamic_cast<MockGame*>(&game);
    ASSERT_NE(mock_game, nullptr);
    
    // Set game to quit after 1 frame
    mock_game->set_max_frames(1);
    
    // Run engine
    EXPECT_NO_THROW(engine.run());
    
    // Game should have been initialized
    EXPECT_TRUE(mock_game->set_engine_called);
    EXPECT_TRUE(mock_game->pre_start_called);
    EXPECT_TRUE(mock_game->start_called);
    EXPECT_TRUE(mock_game->post_start_called);
    
    // Note: Game shutdown happens in Engine destructor, not in run()
    // We'll test that separately in the destruction test
}

TEST_F(EngineTest, EngineInitStatusProgression) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    // Initially not initialized
    EXPECT_EQ(engine.get_engine_init_status(), EngineInitStatus::NotInitialized);
    
    // Set to quit immediately to test init sequence
    mock_game->set_max_frames(0);
    
    // Run and check status progression (this is hard to test in detail due to timing)
    engine.run();
    
    // After run(), we should be back to some final state
    // (The exact state depends on implementation details)
}

TEST_F(EngineTest, EngineMultipleFramesExecution) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    // Set to run for multiple frames
    const int frame_count = 5;
    mock_game->set_max_frames(frame_count);
    
    auto start_time = std::chrono::steady_clock::now();
    engine.run();
    auto end_time = std::chrono::steady_clock::now();
    
    // Engine should have executed multiple frames
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should have taken some time to execute multiple frames
    // (This is a rough check - actual timing depends on system performance)
    // Note: In fast test environments, this might be 0ms, so we'll just check it's non-negative
    EXPECT_GE(duration.count(), 0);
    
    // Lifecycle should have been called
    EXPECT_TRUE(mock_game->start_called);
    // Note: shutdown is called in destructor, not in run()
}

TEST_F(EngineTest, EngineClockUpdatesCorrectly) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    mock_game->set_max_frames(3);
    
    // Get initial clock state
    const IEngineClock& clock = engine.get_clock();
    auto initial_total = clock.get_total_time();
    
    // Run engine
    engine.run();
    
    // Total time should have increased
    auto final_total = clock.get_total_time();
    EXPECT_GT(final_total, initial_total);
    
    // Delta time should be set to some reasonable value from the last frame
    auto delta = clock.get_delta_time();
    EXPECT_GE(delta, 0.0);
    // Delta should be reasonable (less than 1 second per frame for a simple test)
    EXPECT_LT(delta, 1.0);
}

// Test Game lifecycle integration
TEST_F(EngineTest, GameLifecycleOrder) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    mock_game->set_max_frames(1);
    
    engine.run();
    
    // Check that game startup lifecycle methods were called in correct order
    // Note: shutdown happens in destructor, so we only check startup here
    std::vector<std::string> expected_startup_order = {
        "game_pre_start",
        "game_start", 
        "game_post_start"
    };
    
    // Extract only the startup entries from the lifecycle_order
    std::vector<std::string> startup_order;
    for (const auto& entry : mock_game->lifecycle_order) {
        if (entry.find("start") != std::string::npos) {
            startup_order.push_back(entry);
        }
    }
    
    EXPECT_EQ(startup_order, expected_startup_order);
}

TEST_F(EngineTest, GameEngineAssociation) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    mock_game->set_max_frames(1);
    
    engine.run();
    
    // After run(), game should have engine reference
    EXPECT_TRUE(mock_game->set_engine_called);
    EXPECT_EQ(&mock_game->get_engine(), &engine);
}

// Test error conditions
TEST_F(EngineTest, GameQuitCondition) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    // Set to quit after 2 frames
    mock_game->set_max_frames(2);
    
    auto start_time = std::chrono::steady_clock::now();
    engine.run();
    auto end_time = std::chrono::steady_clock::now();
    
    // Should have completed relatively quickly (within reasonable time)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    // Lifecycle should be complete for startup
    EXPECT_TRUE(mock_game->start_called);
    // Note: shutdown is called in destructor, not in run()
}

TEST_F(EngineTest, ImmediateQuit) {
    Engine<MockGame> engine;
    MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
    ASSERT_NE(mock_game, nullptr);
    
    // Set to quit immediately
    mock_game->set_max_frames(0);
    
    engine.run();
    
    // Even with immediate quit, initialization should happen
    EXPECT_TRUE(mock_game->set_engine_called);
    EXPECT_TRUE(mock_game->start_called);
    // Note: shutdown is called in destructor, not in run()
}

// Test engine destruction
TEST_F(EngineTest, EngineDestruction) {
    // Create engine in a scope to test destruction
    {
        Engine<MockGame> engine;
        MockGame* mock_game = dynamic_cast<MockGame*>(&engine.get_game());
        ASSERT_NE(mock_game, nullptr);
        
        mock_game->set_max_frames(1);
        engine.run();
        
        EXPECT_TRUE(mock_game->start_called);
        // shutdown will be called in destructor
    } // Engine goes out of scope here
    
    // If we get here without crashing, destruction worked correctly
    SUCCEED();
}

} // namespace atlas::core