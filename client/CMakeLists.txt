cmake_minimum_required(VERSION 3.20)

project(client)

add_executable(TcpClient main.cpp)
target_link_libraries(TcpClient PUBLIC packetprocess components)

find_package(httplib CONFIG REQUIRED)
add_executable(HttpClient http_client.cpp)
target_link_libraries(HttpClient PUBLIC httplib::httplib components)