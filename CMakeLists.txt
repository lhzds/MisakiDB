cmake_minimum_required(VERSION 3.20)
project(MisakiDB)

set(CMAKE_CXX_STANDARD 20)
#file(GLOB SOURCES
#        "src/*.cpp"
#        "src/file_store/*.cpp"
#        "src/page/*.cpp"
#        "src/page_allocator/*.cpp")
set(SOURCES "src/file_store/file_store.cpp"
        "src/file_store/index_file_store.cpp")

add_library(MisakiDB ${SOURCES})
include_directories("src/include")

add_executable(main test/main.cpp)
target_link_libraries(main MisakiDB)

add_subdirectory(test)


