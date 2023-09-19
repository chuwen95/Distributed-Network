cmake_minimum_required(VERSION 3.20)

project(reactor)

FILE(GLOB_RECURSE SRC *.cpp)

add_library(libreactor ${SRC})
target_link_libraries(libreactor PUBLIC libcomponents libpacketprocess readerwriterqueue)