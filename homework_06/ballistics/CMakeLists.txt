add_library(ballistics STATIC)

target_sources(ballistics PRIVATE include/ballistics/ballistics.h src/ballistics.cpp)

target_include_directories(ballistics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(test)
