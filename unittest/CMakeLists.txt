cmake_minimum_required(VERSION 3.20)

project(unittest)

add_executable(RingBufferTest RingBufferTest.cpp)
target_link_libraries(RingBufferTest PUBLIC libcomponents libpacketprocess)

find_package(httplib CONFIG REQUIRED)

add_executable(HttpLibTest HttpLibTest.cpp)
target_link_libraries(HttpLibTest PUBLIC libcomponents httplib::httplib)