cmake_minimum_required(VERSION 3.30)

# Only define the project if not included as a subdirectory, this is important
# because it won't work if included as a subdirectory from a game project
# otherwise.

# set a variable to check if the project is included as a subdirectory the value
# for the variable should be `CMAKE_CURRENT_SOURCE_DIR STREQUAL
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(BUILDING_ONLY_ATLAS ON)
  message(STATUS "Building Atlas as a standalone project.")
else()
  set(BUILDING_ONLY_ATLAS OFF)
endif()

if(BUILDING_ONLY_ATLAS)
  project(atlas)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(atlas STATIC)
add_library(atlas::atlas ALIAS atlas)
include(SetupModules.cmake SetupExternalLibraries.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC include)
add_subdirectory(include)
add_subdirectory(src)

option(BUILD_TESTS "Build tests" ON)
if(BUILDING_ONLY_ATLAS AND BUILD_TESTS)
  message(STATUS "Building tests.")
  include(CTest)
  add_subdirectory(tests)
else()
  message(STATUS "Skipping tests.")
endif()

# This is false by default, it doesnt make any sense to install this lib on our
# system. This is only used by nix when using the flake.
if(ENABLE_INSTALL AND BUILDING_ONLY_ATLAS)
  install(TARGETS atlas DESTINATION lib)
endif()

if(APPLE)
  target_compile_definitions(atlas PUBLIC VK_USE_PLATFORM_MACOS_MVK
                                          VK_ENABLE_BETA_EXTENSIONS)
elseif(WIN32)
  target_compile_definitions(atlas PUBLIC VK_USE_PLATFORM_WIN32_KHR)
else(UNIX)
  target_compile_definitions(atlas PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
endif()
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
  message(FATAL_ERROR "Vulkan not found")
else()
  target_include_directories(atlas SYSTEM PUBLIC ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(atlas PRIVATE Vulkan::Vulkan)
endif()
