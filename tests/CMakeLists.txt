find_package(GTest CONFIG REQUIRED)

add_executable(atlas_core)

target_link_libraries(atlas_core PRIVATE atlas GTest::gtest_main)
target_sources(atlas_core PRIVATE TestMain.cpp TestEngine.cpp TestModule.cpp TestHephaestusConst.cpp TestSystemConstIntegration.cpp TestEndToEndConst.cpp)

include(GoogleTest)
gtest_discover_tests(atlas_core)
