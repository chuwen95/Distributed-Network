cmake_minimum_required(VERSION 3.20)

# vcpkg init
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    find_package(Git REQUIRED)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- vcpkg WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/deps-cmake" CACHE PATH "The path to the deps cmake directory")

project(TcpNetwork)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR})

include(ProjectReaderWriterQueue)

add_subdirectory(libcomponents)
add_subdirectory(libreactor)
add_subdirectory(libpacketprocess)
add_subdirectory(server)
add_subdirectory(client)

if(DEFINED TEST)
    add_subdirectory(unittest)
endif ()