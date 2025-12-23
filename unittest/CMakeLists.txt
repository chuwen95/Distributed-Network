find_package(GTest CONFIG REQUIRED)

include(GoogleTest)
enable_testing()

set(TARGET GTest::gtest PROPERTY CXX_STANDARD 17)
set(TARGET GTest::gtest_main PROPERTY CXX_STANDARD 17)

add_executable(RingBufferTest RingBufferTest.cpp)
target_link_libraries(RingBufferTest PUBLIC utilities packetprocess service GTest::gtest GTest::gtest_main)

gtest_discover_tests(RingBufferTest)