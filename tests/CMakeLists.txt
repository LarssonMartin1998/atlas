find_package(GTest REQUIRED)

add_executable(all_tests)

target_link_libraries(all_tests PRIVATE ${PROJECT_NAME} GTest::gtest_main)
target_sources(all_tests PRIVATE test_atlas.cpp)

include(GoogleTest)
gtest_discover_tests(all_tests)
