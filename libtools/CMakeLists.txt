cmake_minimum_required(VERSION 3.20)

project(libtools)

find_path(INIPP_INCLUDE_DIRS "inipp.h")
add_library(inipp INTERFACE)
target_include_directories(inipp INTERFACE ${INIPP_INCLUDE_DIRS})

FILE(GLOB_RECURSE SRCS *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

add_library(tools ${SRCS} ${HEDS})
target_link_libraries(tools PUBLIC components inipp)