cmake_minimum_required(VERSION 3.20)

project(telemetry CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# create a static library target for telemetry
add_library(telemetry STATIC)

target_sources(telemetry PRIVATE include/telemetry/telemetry.hpp src/telemetry.cpp)

# make the include directory available to users of the telemetry library
target_include_directories(telemetry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# create an executable target for telemetry_check
add_executable(telemetry_check src/main.cpp)

target_compile_options(telemetry_check PRIVATE -Wall -Wextra -pedantic)

# link the telemetry library to the telemetry_check executable
target_link_libraries(telemetry_check PRIVATE telemetry)
