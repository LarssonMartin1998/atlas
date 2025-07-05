target_include_directories(atlas PUBLIC include)
target_sources(
  atlas PRIVATE src/hephaestus/Hephaestus.cpp src/hephaestus/Archetype.cpp
                src/hephaestus/Utils.cpp)
