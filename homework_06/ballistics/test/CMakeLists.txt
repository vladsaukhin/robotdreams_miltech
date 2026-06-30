add_executable(ballistics-test)

if(NOT CMAKE_CROSSCOMPILING)
    find_package(GTest REQUIRED)
endif()

target_sources(ballistics-test PRIVATE ballistics_tests.cpp input_params_tests.cpp)

target_link_libraries(ballistics-test PRIVATE ballistics GTest::gmock_main)

target_compile_definitions(ballistics-test
                           PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data")

target_compile_definitions(ballistics-test PRIVATE REAL_DATA_DIR="${PROJECT_SOURCE_DIR}/data")

include(GoogleTest)
gtest_discover_tests(ballistics-test DISCOVERY_MODE PRE_TEST)
