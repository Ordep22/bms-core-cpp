include(GoogleTest)

add_executable(
        test_bms test_moving_average_filter.cpp
        test_safety_monitor.cpp

)
target_link_libraries(test_bms PRIVATE GTest::gtest GTest::gtest_main)
target_link_libraries(test_bms PRIVATE bms_core)
gtest_discover_tests(test_bms)