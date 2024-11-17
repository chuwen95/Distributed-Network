cmake_minimum_required(VERSION 3.20)

# vcpkg init
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    find_package(Git REQUIRED)
    execute_process(COMMAND git submodule update --init --recursive -- vcpkg WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND git checkout master WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vcpkg")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the deps cmake directory")

project(CopyStateMachine)

set(CMAKE_CXX_STANDARD 20)

include(IncludeDirectories)
include(ProjectReaderWriterQueue)

add_subdirectory(csm-utilities)
add_subdirectory(csm-rpc)
add_subdirectory(csm-consensus)
add_subdirectory(csm-service)
add_subdirectory(csm-packetprocess)
add_subdirectory(csm-tool)
add_subdirectory(csm-stmclog)
add_subdirectory(csm-storage)
add_subdirectory(csm-initializer)
add_subdirectory(server)
add_subdirectory(client)

if(DEFINED TEST)
    add_subdirectory(unittest)
endif ()