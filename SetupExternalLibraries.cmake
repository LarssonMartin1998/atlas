find_package(Taskflow 3.9.0 REQUIRED)

target_link_libraries(atlas PRIVATE Taskflow::Taskflow)
