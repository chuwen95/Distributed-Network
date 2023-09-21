cmake_minimum_required(VERSION 3.20)

project(reactor)

FILE(GLOB_RECURSE SRC *.cpp)

find_path(INIPP_INCLUDE_DIRS "inipp.h")
add_library(libinipp INTERFACE)
target_include_directories(libinipp INTERFACE ${INIPP_INCLUDE_DIRS})

add_library(libreactor ${SRC})
target_link_libraries(libreactor PUBLIC libcomponents libpacketprocess readerwriterqueue libinipp)