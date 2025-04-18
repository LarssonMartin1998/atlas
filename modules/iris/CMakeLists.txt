target_include_directories(atlas PUBLIC include)
add_subdirectory(src)

include(SetupVulkan.cmake)
include(SetupGlfw.cmake)
