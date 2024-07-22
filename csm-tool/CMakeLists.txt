find_path(INIPP_INCLUDE_DIRS "inipp.h")
add_library(inipp INTERFACE)
target_include_directories(inipp INTERFACE ${INIPP_INCLUDE_DIRS})

FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(tools ${SOURCES} ${HEADERS})
target_link_libraries(tools PUBLIC utilities inipp)