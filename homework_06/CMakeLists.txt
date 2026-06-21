cmake_minimum_required(VERSION 3.20)

project(ballistics_core CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(ballistics)

add_executable(ballistics_core src/main.cpp)

target_compile_options(ballistics_core PRIVATE -Wall -Wextra -pedantic)

target_link_libraries(ballistics_core PRIVATE ballistics)
