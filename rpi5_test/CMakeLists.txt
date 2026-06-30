cmake_minimum_required(VERSION 3.20)

project(hello_arm LANGUAGES CXX)

add_executable(hello_arm main.cpp)

target_compile_features(hello_arm PRIVATE cxx_std_20)
