include(GoogleTest)

add_executable(test_bms test_bms_fsm.cpp
        test_moving_average_filter.cpp)
target_link_libraries(test_bms PRIVATE GTest::gtest GTest::gtest_main)

gtest_discover_tests(test_bms)