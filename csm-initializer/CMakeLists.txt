FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(initializer ${SOURCES} ${HEADERS}
        StateMachineLogInitializer.cpp
        StateMachineLogInitializer.h
)
target_link_libraries(initializer PUBLIC rpc service storage raft)