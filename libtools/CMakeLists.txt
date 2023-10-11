cmake_minimum_required(VERSION 3.20)

project(tools)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

find_path(INIPP_INCLUDE_DIRS "inipp.h")
add_library(libinipp INTERFACE)
target_include_directories(libinipp INTERFACE ${INIPP_INCLUDE_DIRS})

add_library(libtools ${SRC} ${HEDS})
target_link_libraries(libtools PUBLIC libcomponents libinipp)