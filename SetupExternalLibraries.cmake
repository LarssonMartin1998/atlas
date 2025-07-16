# External library dependencies
find_package(Taskflow 3.10.0 REQUIRED)

target_link_libraries(atlas PRIVATE Taskflow::Taskflow)

# GTest is only needed when building tests
if(BUILD_TESTS)
  find_package(GTest CONFIG REQUIRED)
endif()
