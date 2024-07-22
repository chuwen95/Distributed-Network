FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

find_package(RocksDB CONFIG REQUIRED)

add_library(storage ${SOURCES} ${HEADERS})
target_link_libraries(storage PUBLIC RocksDB::rocksdb utilities)