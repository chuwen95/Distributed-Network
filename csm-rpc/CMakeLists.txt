FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

find_package(httplib CONFIG REQUIRED)

add_library(rpc ${SOURCES} ${HEADERS}
        tcp/TcpRpcServer.cpp
        tcp/TcpRpcServer.h
        http/HttpRpcServer.cpp)
target_link_libraries(rpc PUBLIC httplib::httplib service packetprocess)