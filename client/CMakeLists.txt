cmake_minimum_required(VERSION 3.20)

project(client)

add_executable(TcpClient main.cpp)
target_link_libraries(TcpClient PUBLIC libpacketprocess libcomponents)