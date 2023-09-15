cmake_minimum_required(VERSION 3.25)
project(server)

add_executable(TcpServer main.cpp)
target_link_libraries(TcpServer PUBLIC libreactor libpacketprocess)