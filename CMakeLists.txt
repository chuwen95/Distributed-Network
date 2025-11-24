cmake_minimum_required(VERSION 3.20)

# vcpkg init
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vcpkg")
    find_package(Git REQUIRED)
    execute_process(COMMAND ${GIT_EXECUTABLE} clone --single-branch --branch master --depth 1
            https://github.com/microsoft/vcpkg.git vcpkg WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND git checkout master WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vcpkg")
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the deps cmake directory")

project(CopyStateMachine)

set(CMAKE_CXX_STANDARD 20)

# 检测GCC版本是否大于8.1 或 Clang版本是否大于9.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.1)
        message(STATUS "Using GCC >= 8.1, enabling -fmacro-prefix-map")
        add_compile_options(-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0)
        message(STATUS "Using Clang >= 9.0, enabling -fmacro-prefix-map")
        add_compile_options(-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.)
    endif()
else()
    message(STATUS "Compiler is not GCC or Clang, skipping -fmacro-prefix-map")
endif()

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