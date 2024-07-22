FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(initializer ${SOURCES} ${HEADERS})
target_link_libraries(initializer PUBLIC rpc service storage consensus)