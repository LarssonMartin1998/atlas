#include <gtest/gtest.h>

class TestEngine : public ::testing::Test {
  protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TestEngine, ExampleTest) { EXPECT_TRUE(true); }
