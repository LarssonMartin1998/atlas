cmake_minimum_required(VERSION 3.28)
project(atlas)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE src/atlas/atlas.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE gtest::gtest)
