cmake_minimum_required(VERSION 3.20)

project(libinitializer)

FILE(GLOB_RECURSE SRC *.cpp)

add_library(libinitializer ${SRC})
target_link_libraries(libinitializer PUBLIC libservice)