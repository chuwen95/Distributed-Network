cmake_minimum_required(VERSION 3.20)

project(librpc)

FILE(GLOB_RECURSE SRCS *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

find_package(httplib CONFIG REQUIRED)

add_library(rpc ${SRCS} ${HEDS})
target_link_libraries(rpc PUBLIC httplib::httplib service packetprocess)