add_library(ballistics STATIC)

target_sources(
    ballistics PRIVATE include/ballistics/ballistics.h include/ballistics/input_params.h
                       include/ballistics/coord.h src/ballistics.cpp src/input_params.cpp)

target_include_directories(ballistics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT CMAKE_CROSSCOMPILING)

    add_subdirectory(test)

endif()
