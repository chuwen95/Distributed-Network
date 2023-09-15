cmake_minimum_required(VERSION 3.20)

project(components)

FILE(GLOB_RECURSE SRC *.cpp)
FILE(GLOB_RECURSE HED *.h)

add_library(libcomponents ${SRC} ${HED})