#include <gtest/gtest.h>

class TestModule : public ::testing::Test {
  protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TestModule, ExampleTest) { EXPECT_TRUE(true); }
