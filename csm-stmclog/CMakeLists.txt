FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(stmclog ${SOURCES} ${HEADERS}
        StateMachineLogFactory.cpp
        StateMachineLogFactory.h
        StateMachineLogFactory.h
        StateMachineLog.cpp)