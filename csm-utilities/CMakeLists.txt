FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

find_package(concurrentqueue CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

add_library(utilities ${SOURCES} ${HEADERS})
target_link_libraries(utilities PUBLIC concurrentqueue::concurrentqueue stduuid)