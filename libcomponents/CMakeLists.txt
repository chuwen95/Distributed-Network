cmake_minimum_required(VERSION 3.20)

project(libcomponents)

FILE(GLOB_RECURSE SRCS *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

add_library(components ${SRCS} ${HEDS})
target_link_libraries(components PUBLIC unofficial::concurrentqueue::concurrentqueue stduuid)