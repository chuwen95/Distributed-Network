cmake_minimum_required(VERSION 3.25)
project(server)

add_executable(TcpService main.cpp)
target_link_libraries(TcpService PUBLIC initializer)