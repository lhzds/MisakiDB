cmake_minimum_required(VERSION 3.20)
project(MisakiDB)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fconcepts)

file(GLOB SOURCES
        "src/*.cpp"
        "src/*/*.cpp")

add_library(MisakiDB ${SOURCES})
add_executable(main "src/main.cpp")
target_link_libraries(main MisakiDB ws2_32)

include_directories("src/include")
add_subdirectory(test)
