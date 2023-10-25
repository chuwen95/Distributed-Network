cmake_minimum_required(VERSION 3.20)

project(libinitializer)

FILE(GLOB_RECURSE SRCS *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

add_library(initializer ${SRCS} ${HEDS})
target_link_libraries(initializer PUBLIC rpc service)