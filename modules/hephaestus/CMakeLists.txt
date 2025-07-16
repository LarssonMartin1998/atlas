# Taskflow is required for Hephaestus functionality
find_package(Taskflow 3.10.0 REQUIRED)
target_link_libraries(atlas PRIVATE Taskflow::Taskflow)

target_include_directories(atlas PUBLIC include)
target_sources(
  atlas PRIVATE src/hephaestus/Hephaestus.cpp src/hephaestus/Archetype.cpp
                src/hephaestus/Utils.cpp)
