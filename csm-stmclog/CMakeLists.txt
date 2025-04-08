# proto生成c++文件
set(module_path ${CMAKE_CURRENT_SOURCE_DIR})
set(proto_output_path ${CMAKE_CURRENT_BINARY_DIR})

# 填充.proto文件
set(proto_files protocol/pb/entry.proto)
# 获取libservice文件夹的绝对路径
get_filename_component(module_absolute_path ${module_path} ABSOLUTE)
# 遍历.proto文件
foreach (proto_file ${proto_files})
    set(proto_file_absolute_path ${module_absolute_path}/${proto_file})

    get_filename_component(dir_part ${proto_file} DIRECTORY)
    get_filename_component(filename ${proto_file} NAME_WE)
    set(generate_file ${proto_output_path}/${dir_part}/${filename}.pb.cc)

    list(APPEND PROTOBUF_SRCS ${generate_file})

    message("Command: protoc --cpp_out ${proto_output_path} -I ${module_path} ${proto_file}")
    add_custom_command(
            OUTPUT ${generate_file}
            COMMAND protobuf::protoc --cpp_out ${proto_output_path} -I ${module_path} ${proto_file}
            COMMENT "Generating ${proto_generate_srcs} from ${proto_file_absolute_path}"
            VERBATIM
    )
endforeach ()

find_package(Protobuf CONFIG REQUIRED)

FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

add_library(stmclog ${SOURCES} ${PROTOBUF_SRCS} ${HEADERS})
target_link_libraries(stmclog PUBLIC storage protobuf::libprotobuf)
target_include_directories(stmclog BEFORE PUBLIC ${proto_output_path})