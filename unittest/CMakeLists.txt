cmake_minimum_required(VERSION 3.20)

project(unittest)

add_executable(RingBufferTest RingBufferTest.cpp)
target_link_libraries(RingBufferTest PUBLIC libcomponents libpacketprocess)