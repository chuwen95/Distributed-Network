find_package(httplib CONFIG REQUIRED)
add_executable(HttpClient http_client.cpp)
target_link_libraries(HttpClient PUBLIC httplib::httplib utilities)