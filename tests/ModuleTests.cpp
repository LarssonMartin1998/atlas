#include <gtest/gtest.h>
#include <atlas/core/Module.hpp>
#include <atlas/core/IModule.hpp>
#include <atlas/core/IEngine.hpp>
#include <atlas/core/ITickable.hpp>
#include <atlas/core/IStartShutdown.hpp>
#include <atlas/core/IEngineHandle.hpp>

namespace atlas::core {

// Mock Engine for testing
class MockEngine : public IEngine {
public:
    MockEngine() = default;
    
    auto run() -> void override {
        run_called = true;
    }
    
    auto get_game() -> IGame& override {
        throw std::runtime_error("Not implemented for mock");
    }
    
    auto get_clock() const -> const IEngineClock& override {
        throw std::runtime_error("Not implemented for mock");
    }
    
    auto get_engine_init_status() const -> EngineInitStatus override {
        return init_status;
    }
    
    // Test helpers
    bool run_called = false;
    EngineInitStatus init_status = EngineInitStatus::NotInitialized;
    
protected:
    auto get_module_impl(std::type_index module) const -> IModule* override {
        return nullptr;
    }
};

// Test Module to verify lifecycle calls
class TestModule : public Module {
public:
    explicit TestModule(IEngine& engine) : Module(engine) {}
    
    auto pre_start() -> void override {
        pre_start_called = true;
        call_order.push_back("pre_start");
        Module::pre_start();
    }
    
    auto start() -> void override {
        start_called = true;
        call_order.push_back("start");
        Module::start();
    }
    
    auto post_start() -> void override {
        post_start_called = true;
        call_order.push_back("post_start");
        Module::post_start();
    }
    
    auto pre_shutdown() -> void override {
        pre_shutdown_called = true;
        call_order.push_back("pre_shutdown");
        Module::pre_shutdown();
    }
    
    auto shutdown() -> void override {
        shutdown_called = true;
        call_order.push_back("shutdown");
        Module::shutdown();
    }
    
    auto post_shutdown() -> void override {
        post_shutdown_called = true;
        call_order.push_back("post_shutdown");
        Module::post_shutdown();
    }
    
    // Test state
    bool pre_start_called = false;
    bool start_called = false;
    bool post_start_called = false;
    bool pre_shutdown_called = false;
    bool shutdown_called = false;
    bool post_shutdown_called = false;
    std::vector<std::string> call_order;
};

// Tickable Test Module
class TickableTestModule : public Module, public ITickable {
public:
    explicit TickableTestModule(IEngine& engine) : Module(engine) {}
    
    auto tick() -> void override {
        tick_called = true;
        tick_count++;
    }
    
    auto get_tick_rate() const -> unsigned override {
        return tick_rate;
    }
    
    // Test state
    bool tick_called = false;
    unsigned tick_count = 0;
    unsigned tick_rate = 60; // Default 60 Hz
};

class ModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_engine = std::make_unique<MockEngine>();
    }
    
    void TearDown() override {
        mock_engine.reset();
    }
    
    std::unique_ptr<MockEngine> mock_engine;
};

// Test Module construction and basic functionality
TEST_F(ModuleTest, ModuleConstruction) {
    TestModule module(*mock_engine);
    
    // Module should store engine reference correctly
    EXPECT_EQ(&module.get_engine(), mock_engine.get());
}

TEST_F(ModuleTest, ModuleLifecycleMethods) {
    TestModule module(*mock_engine);
    
    // Initially, no lifecycle methods should be called
    EXPECT_FALSE(module.pre_start_called);
    EXPECT_FALSE(module.start_called);
    EXPECT_FALSE(module.post_start_called);
    EXPECT_FALSE(module.pre_shutdown_called);
    EXPECT_FALSE(module.shutdown_called);
    EXPECT_FALSE(module.post_shutdown_called);
    
    // Call startup sequence
    module.pre_start();
    EXPECT_TRUE(module.pre_start_called);
    EXPECT_FALSE(module.start_called);
    
    module.start();
    EXPECT_TRUE(module.start_called);
    EXPECT_FALSE(module.post_start_called);
    
    module.post_start();
    EXPECT_TRUE(module.post_start_called);
    
    // Call shutdown sequence
    module.pre_shutdown();
    EXPECT_TRUE(module.pre_shutdown_called);
    EXPECT_FALSE(module.shutdown_called);
    
    module.shutdown();
    EXPECT_TRUE(module.shutdown_called);
    EXPECT_FALSE(module.post_shutdown_called);
    
    module.post_shutdown();
    EXPECT_TRUE(module.post_shutdown_called);
}

TEST_F(ModuleTest, LifecycleCallOrder) {
    TestModule module(*mock_engine);
    
    // Execute full lifecycle
    module.pre_start();
    module.start();
    module.post_start();
    module.pre_shutdown();
    module.shutdown();
    module.post_shutdown();
    
    // Verify correct order
    std::vector<std::string> expected_order = {
        "pre_start", "start", "post_start", 
        "pre_shutdown", "shutdown", "post_shutdown"
    };
    
    EXPECT_EQ(module.call_order, expected_order);
}

TEST_F(ModuleTest, MultipleModuleInstances) {
    TestModule module1(*mock_engine);
    TestModule module2(*mock_engine);
    
    // Both modules should reference the same engine
    EXPECT_EQ(&module1.get_engine(), &module2.get_engine());
    EXPECT_EQ(&module1.get_engine(), mock_engine.get());
    
    // Modules should be independent
    module1.start();
    EXPECT_TRUE(module1.start_called);
    EXPECT_FALSE(module2.start_called);
    
    module2.pre_start();
    EXPECT_TRUE(module2.pre_start_called);
    EXPECT_FALSE(module1.pre_start_called);
}

// Test ITickable interface implementation
TEST_F(ModuleTest, TickableModuleBasicFunctionality) {
    TickableTestModule module(*mock_engine);
    
    // Initially not ticked
    EXPECT_FALSE(module.tick_called);
    EXPECT_EQ(module.tick_count, 0);
    
    // Tick the module
    module.tick();
    EXPECT_TRUE(module.tick_called);
    EXPECT_EQ(module.tick_count, 1);
    
    // Tick again
    module.tick();
    EXPECT_EQ(module.tick_count, 2);
}

TEST_F(ModuleTest, TickableModuleTickRate) {
    TickableTestModule module(*mock_engine);
    
    // Default tick rate should be 60
    EXPECT_EQ(module.get_tick_rate(), 60);
    
    // Change tick rate
    module.tick_rate = 120;
    EXPECT_EQ(module.get_tick_rate(), 120);
    
    module.tick_rate = 30;
    EXPECT_EQ(module.get_tick_rate(), 30);
}

TEST_F(ModuleTest, TickableModuleMultipleTicks) {
    TickableTestModule module(*mock_engine);
    
    const unsigned tick_count = 100;
    for (unsigned i = 0; i < tick_count; ++i) {
        module.tick();
    }
    
    EXPECT_EQ(module.tick_count, tick_count);
    EXPECT_TRUE(module.tick_called);
}

// Test interface inheritance
TEST_F(ModuleTest, ModuleInterfaceInheritance) {
    TestModule module(*mock_engine);
    
    // Module should implement all required interfaces
    IModule* i_module = &module;
    IStartShutdown* i_start_shutdown = &module;
    IEngineHandle* i_engine_handle = &module;
    
    EXPECT_NE(i_module, nullptr);
    EXPECT_NE(i_start_shutdown, nullptr);
    EXPECT_NE(i_engine_handle, nullptr);
    
    // All interfaces should provide access to the same engine
    EXPECT_EQ(&i_engine_handle->get_engine(), mock_engine.get());
    EXPECT_EQ(&i_module->get_engine(), mock_engine.get());
}

TEST_F(ModuleTest, TickableModuleInterfaceInheritance) {
    TickableTestModule module(*mock_engine);
    
    // TickableTestModule should implement all required interfaces
    IModule* i_module = &module;
    ITickable* i_tickable = &module;
    IStartShutdown* i_start_shutdown = &module;
    IEngineHandle* i_engine_handle = &module;
    
    EXPECT_NE(i_module, nullptr);
    EXPECT_NE(i_tickable, nullptr);
    EXPECT_NE(i_start_shutdown, nullptr);
    EXPECT_NE(i_engine_handle, nullptr);
    
    // Tickable interface should work correctly
    i_tickable->tick();
    EXPECT_TRUE(module.tick_called);
    EXPECT_EQ(i_tickable->get_tick_rate(), 60);
}

// Test error conditions and edge cases
TEST_F(ModuleTest, ModuleWithInvalidEngine) {
    // This test would be difficult without changing the Module interface
    // but we can at least verify that valid engines work correctly
    TestModule module(*mock_engine);
    
    // Engine should be accessible
    EXPECT_NO_THROW(module.get_engine());
    EXPECT_EQ(&module.get_engine(), mock_engine.get());
}

TEST_F(ModuleTest, ModuleLifecycleCalledMultipleTimes) {
    TestModule module(*mock_engine);
    
    // Call start multiple times
    module.start();
    module.start();
    module.start();
    
    // Should have been called multiple times
    EXPECT_TRUE(module.start_called);
    EXPECT_EQ(std::count(module.call_order.begin(), module.call_order.end(), "start"), 3);
    
    // Same for shutdown
    module.shutdown();
    module.shutdown();
    
    EXPECT_TRUE(module.shutdown_called);
    EXPECT_EQ(std::count(module.call_order.begin(), module.call_order.end(), "shutdown"), 2);
}

} // namespace atlas::core