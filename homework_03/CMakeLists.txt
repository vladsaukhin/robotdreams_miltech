cmake_minimum_required(VERSION 3.20)

project(homework3 CXX)

# Код домашньої роботи збирається з тим самим стандартом, що й лекційні приклади.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(homework3 src/main.cpp)

target_compile_options(homework3 PRIVATE -Wall -Wextra -pedantic)

find_package(nlohmann_json REQUIRED)

target_link_libraries(homework3 PRIVATE nlohmann_json::nlohmann_json)
