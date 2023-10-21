cmake_minimum_required(VERSION 3.20)

project(libinitializer)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

add_library(libinitializer ${SRC} ${HEDS})
target_link_libraries(libinitializer PUBLIC librpc libservice)