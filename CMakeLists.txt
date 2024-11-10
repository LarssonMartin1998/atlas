cmake_minimum_required(VERSION 3.30)
project(atlas)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(include)
add_subdirectory(src)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
else()
  message(STATUS "BUILD_TESTS=OFF, skipping tests.")
endif()

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}_target.cmake)
