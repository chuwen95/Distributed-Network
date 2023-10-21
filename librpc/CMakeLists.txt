cmake_minimum_required(VERSION 3.20)

project(rpc)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

find_package(httplib CONFIG REQUIRED)

add_library(librpc ${SRC} ${HEDS})
target_link_libraries(librpc PUBLIC httplib::httplib libservice libtools)