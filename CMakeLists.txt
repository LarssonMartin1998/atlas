cmake_minimum_required(VERSION 3.28)
project(atlas)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PUBLIC include)

include(CTest)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(tests)
