cmake_minimum_required(VERSION 3.20)

project(reactor)

FILE(GLOB_RECURSE SRC *.cpp)

find_path(INIPP_INCLUDE_DIRS "inipp.h")
add_library(libinipp INTERFACE)
target_include_directories(libinipp INTERFACE ${INIPP_INCLUDE_DIRS})

find_package(jsoncpp CONFIG REQUIRED)

add_library(libservice ${SRC})
target_link_libraries(libservice PUBLIC libcomponents libpacketprocess readerwriterqueue libinipp JsonCpp::JsonCpp)