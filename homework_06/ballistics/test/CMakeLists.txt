add_executable(ballistics-test)

target_sources(ballistics-test PRIVATE ballistics_tests.cpp)

target_link_libraries(ballistics-test PRIVATE ballistics GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(ballistics-test DISCOVERY_MODE PRE_TEST)
