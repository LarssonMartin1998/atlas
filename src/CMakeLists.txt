find_package(RAYLIB REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

target_sources(${PROJECT_NAME} PRIVATE atlas/atlas.cpp)
