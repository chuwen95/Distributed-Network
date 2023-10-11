cmake_minimum_required(VERSION 3.20)

project(reactor)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HEDS *.h)

find_package(jsoncpp CONFIG REQUIRED)

add_library(libservice ${SRC} ${HEDS})
target_link_libraries(libservice PUBLIC libtools libpacketprocess readerwriterqueue JsonCpp::JsonCpp)