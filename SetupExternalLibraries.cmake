find_package(Taskflow REQUIRED)

target_link_libraries(atlas PRIVATE Taskflow::Taskflow)
