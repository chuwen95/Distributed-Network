cmake_minimum_required(VERSION 3.20)

project(components)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HED *.h)

find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

add_library(libcomponents ${SRC} ${HED})
target_link_libraries(libcomponents PUBLIC unofficial::concurrentqueue::concurrentqueue stduuid)